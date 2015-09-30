/*
  Lua module to read Android sensors (accelerometer, etc.)
*/

#include <android/sensor.h>
#include <android/looper.h>
#include <android/log.h>

#ifdef __cplusplus
extern "C"
{
#endif
  #include "lua.h"
  #include "lauxlib.h"
#ifdef __cplusplus
}
#endif

#include "luasensor.h"

#define MT_NAME "sensor_mt"
#define MAX_NUM_EVENTS 32

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef struct luaIntConst {
  const char *key;
  int value;
} luaIntConst;

static luaIntConst luaSensorConst[] = {
  {"TYPE_ACCELEROMETER", ASENSOR_TYPE_ACCELEROMETER},
  {"TYPE_MAGNETIC_FIELD", ASENSOR_TYPE_MAGNETIC_FIELD},
  {"TYPE_GYROSCOPE", ASENSOR_TYPE_GYROSCOPE},
  {"TYPE_LIGHT", ASENSOR_TYPE_LIGHT},
  {"TYPE_PROXIMITY", ASENSOR_TYPE_PROXIMITY},
  {"STATUS_UNRELIABLE", ASENSOR_STATUS_UNRELIABLE},
  {"STATUS_ACCURACY_LOW", ASENSOR_STATUS_ACCURACY_LOW},
  {"STATUS_ACCURACY_MEDIUM", ASENSOR_STATUS_ACCURACY_MEDIUM},
  {"STATUS_ACCURACY_HIGH", ASENSOR_STATUS_ACCURACY_HIGH},
  { NULL, 0}
};

int sensorCallback(int fd, int events, void *data);

class SensorClass {
public:
  ASensorManager* manager;
  ALooper* looper;
  ASensorEventQueue* eventQueue;

  ASensorList list;
  int nList;

  int ident;
  int nCallback;

  const ASensor* accelerometerSensor;
  SensorClass() : nCallback(0) {
    // Get singleton SensorManager
    manager = ASensorManager_getInstance();
    // List and number of available sensors
    nList = ASensorManager_getSensorList(manager, &list);

    looper = ALooper_forThread();
    if (looper == NULL) {
      looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }
    

    // If sensorCallback() is to be called,
    // need to involve ALooper_pollOnce() or ALooper_pollAll()
    /*
    ident = ALOOPER_POLL_CALLBACK;
    eventQueue =
      ASensorManager_createEventQueue(manager, looper,
				      ident, sensorCallback, this);
    */
    ident = 1;
    eventQueue =
      ASensorManager_createEventQueue(manager, looper,
				      ident, NULL, this);
  }
  virtual ~SensorClass() {
    LOGI("SensorClass destructor");
    if (eventQueue) {
      ASensorManager_destroyEventQueue(manager, eventQueue);
      eventQueue = 0;
    }
  }
};

// sensorCallback() is not used since looper still needs to be polled
// Instead we are polling directly via hasEvents(), getEvents()
int sensorCallback(int fd, int events, void *data) {
  SensorClass *sensor = (SensorClass *)data;
  sensor->nCallback++;
  if (sensor->nCallback % 10 == 0) {
    LOGI("sensorCallback %d: %d %d", sensor->nCallback, fd, events);
  }
  ASensorEvent event;
  if (ASensorEventQueue_getEvents(sensor->eventQueue,
				  &event, 1) > 0) {
    LOGI("sensor %d type %d",event.sensor, event.type);
  }
  // Continue receiving callbacks:  
  return 1;
}

static SensorClass* lua_checksensorclass(lua_State *L, int narg) {
  void *ud = luaL_checkudata(L, narg, MT_NAME);
  luaL_argcheck(L, *(SensorClass **)ud != NULL, narg, "invalid object");
  return *(SensorClass **)ud;
}

static int lua_sensor_new(lua_State *L) {
  SensorClass **ud =
    (SensorClass **)lua_newuserdata(L, sizeof(SensorClass *));
  *ud = new SensorClass();
  luaL_getmetatable(L, MT_NAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int lua_sensor_getSensorList(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);

  lua_createtable(L, sensor->nList, 0);
  for (int i = 0; i < sensor->nList; i++) {
    const ASensor* a = sensor->list[i];
    lua_createtable(L, 0, 5);

    lua_pushliteral(L, "name");
    lua_pushstring(L, ASensor_getName(a));
    lua_rawset(L, -3);
    lua_pushliteral(L, "vendor");
    lua_pushstring(L, ASensor_getVendor(a));
    lua_rawset(L, -3);
    lua_pushliteral(L, "type");
    lua_pushinteger(L, ASensor_getType(a));
    lua_rawset(L, -3);
    lua_pushliteral(L, "resolution");
    lua_pushnumber(L, ASensor_getResolution(a));
    lua_rawset(L, -3);
    lua_pushliteral(L, "minDelay");
    lua_pushinteger(L, ASensor_getMinDelay(a));
    lua_rawset(L, -3);

    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

static int lua_sensor_getDefaultSensor(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  int type = luaL_checkint(L, 2);
  const ASensor* s =
    ASensorManager_getDefaultSensor(sensor->manager, type);
  if (s == NULL) return 0;

  for (int i = 0; i < sensor->nList; i++) {
    // Search sensor list for default sensor index
    if (s == sensor->list[i]) {
      // Convert to 1-index:
      lua_pushinteger(L, i+1);
      return 1;
    }
  }
  return 0;
}

static int lua_sensor_getMinDelay(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  int index = luaL_checkint(L, 2) - 1;
  if ((index >= sensor->nList) || (index < 0))
    return luaL_error(L, "invalid sensor index");
  const ASensor* s = sensor->list[index];

  lua_pushinteger(L, ASensor_getMinDelay(s));
  return 1;
}

static int lua_sensor_enableSensor(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  int nargs = lua_gettop(L);
  for (int iarg = 2; iarg <= nargs; iarg++) {
    // Convert to 0-index:
    int index = luaL_checkint(L, iarg) - 1;
    if ((index >= sensor->nList) || (index < 0))
      return luaL_error(L, "invalid sensor index");
    const ASensor* s = sensor->list[index];

    int ret = ASensorEventQueue_enableSensor(sensor->eventQueue, s);
    lua_pushinteger(L, ret);
  }
  return nargs-1;
}

static int lua_sensor_disableSensor(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  int nargs = lua_gettop(L);
  for (int iarg = 2; iarg <= nargs; iarg++) {
    int index = luaL_checkint(L, iarg) - 1;
    if ((index >= sensor->nList) || (index < 0))
      return luaL_error(L, "invalid sensor index");
    const ASensor* s = sensor->list[index];

    int ret = ASensorEventQueue_disableSensor(sensor->eventQueue, s);
    lua_pushinteger(L, ret);
  }
  return nargs-1;
}

static int lua_sensor_setEventRate(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  int index = luaL_checkint(L, 2) - 1;
  if ((index >= sensor->nList) || (index < 0))
    return luaL_error(L, "invalid sensor index");
  const ASensor* s = sensor->list[index];

  int minDelay = ASensor_getMinDelay(s);
  int usec = luaL_optinteger(L, 3, minDelay);

  int ret = ASensorEventQueue_setEventRate(sensor->eventQueue, s, usec);
  lua_pushinteger(L, ret);
  return 1;
}

static int lua_sensor_hasEvents(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  int hasEvents = ASensorEventQueue_hasEvents(sensor->eventQueue);
  lua_pushboolean(L, hasEvents);
  return 1;
}

static int lua_sensor_getEvents(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);

  ASensorEvent eventBuffer[MAX_NUM_EVENTS];
  int numEvent = ASensorEventQueue_getEvents(sensor->eventQueue,
					     eventBuffer, MAX_NUM_EVENTS);
  if (numEvent < 0) {
    //return luaL_error(L, "getEvents error %d", numEvent);
    lua_pushboolean(L, 0);
    lua_pushinteger(L, numEvent);
    return 2;
  }

  lua_createtable(L, numEvent, 0);
  for (int i = 0; i < numEvent; i++) {
    ASensorEvent* event = eventBuffer + i;
    
    lua_createtable(L, 3, 4);
    /*
    lua_pushliteral(L, "version");
    lua_pushinteger(L, event->version);
    lua_rawset(L, -3);
    */
    lua_pushliteral(L, "sensor");
    // Convert to 1-index:
    lua_pushinteger(L, event->sensor+1);
    lua_rawset(L, -3);
    lua_pushliteral(L, "type");
    lua_pushinteger(L, event->type);
    lua_rawset(L, -3);
    lua_pushliteral(L, "timestamp");
    lua_pushnumber(L, event->timestamp);
    lua_rawset(L, -3);
    switch (event->type) {
    case ASENSOR_TYPE_ACCELEROMETER:
      lua_pushnumber(L, event->acceleration.x);
      lua_rawseti(L, -2, 1);
      lua_pushnumber(L, event->acceleration.y);
      lua_rawseti(L, -2, 2);
      lua_pushnumber(L, event->acceleration.z);
      lua_rawseti(L, -2, 3);
      break;
    case ASENSOR_TYPE_MAGNETIC_FIELD:
      lua_pushnumber(L, event->magnetic.x);
      lua_rawseti(L, -2, 1);
      lua_pushnumber(L, event->magnetic.y);
      lua_rawseti(L, -2, 2);
      lua_pushnumber(L, event->magnetic.z);
      lua_rawseti(L, -2, 3);
      break;
    case ASENSOR_TYPE_GYROSCOPE:
      lua_pushnumber(L, event->vector.azimuth);
      lua_rawseti(L, -2, 1);
      lua_pushnumber(L, event->vector.pitch);
      lua_rawseti(L, -2, 2);
      lua_pushnumber(L, event->vector.roll);
      lua_rawseti(L, -2, 3);
      break;
    case ASENSOR_TYPE_LIGHT:
      lua_pushnumber(L, event->light);
      lua_rawseti(L, -2, 1);
      break;
    case ASENSOR_TYPE_PROXIMITY:
      lua_pushnumber(L, event->distance);
      lua_rawseti(L, -2, 1);
      break;
    default:
      lua_pushnumber(L, event->data[0]);
      lua_rawseti(L, -2, 1);
      lua_pushnumber(L, event->data[1]);
      lua_rawseti(L, -2, 2);
      lua_pushnumber(L, event->data[2]);
      lua_rawseti(L, -2, 3);
    }

    // Set table in events table
    lua_rawseti(L, -2, i+1);
  }

  return 1;
}


static int lua_sensor_delete(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  delete sensor;
  return 0;
}

static int lua_sensor_tostring(lua_State *L) {
  SensorClass *sensor = lua_checksensorclass(L, 1);
  lua_pushfstring(L, "Sensor(%p): %d sensors",
		  sensor, sensor->nList);
  return 1;
}

static const struct luaL_reg sensor_functions[] = {
  {"new", lua_sensor_new},
  {NULL, NULL}
};

static const struct luaL_reg sensor_methods[] = {
  {"getSensorList", lua_sensor_getSensorList},
  {"getDefaultSensor", lua_sensor_getDefaultSensor},
  {"enableSensor", lua_sensor_enableSensor},
  {"disableSensor", lua_sensor_disableSensor},
  {"setEventRate", lua_sensor_setEventRate},
  {"hasEvents", lua_sensor_hasEvents},
  {"getEvents", lua_sensor_getEvents},
  {"getMinDelay", lua_sensor_getMinDelay},
  {"__gc", lua_sensor_delete},
  {"__tostring", lua_sensor_tostring},
  {NULL, NULL}
};

#ifdef __cplusplus
extern "C"
#endif
int luaopen_sensor (lua_State *L) {
  LOGI("luaopen_sensor called");

  luaL_newmetatable(L, MT_NAME);
  // OO access: mt.__index = mt
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaL_register(L, NULL, sensor_methods);

  luaL_newlib(L, sensor_functions);

  // Initialize constants
  luaIntConst *c = luaSensorConst;
  for (; c->key; c++) {
    lua_pushstring(L, c->key);
    lua_pushinteger(L, c->value);
    lua_settable(L, -3);
  }

  return 1;
}
