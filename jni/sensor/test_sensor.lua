sensor = require('sensor')

if not s then
   s = sensor.new();
end

slist = s:getSensorList();
for i = 1,#slist do
   print("Sensor", i);
   for k,v in pairs(slist[i]) do
      print(string.format("  %s %s",k,tostring(v)));
   end
end

print("Default Accel", s:getDefaultSensor(sensor.TYPE_ACCELEROMETER));

for i = 1,#slist do
   s:enableSensor(i)
end

print("proximity", s:setEventRate(9, 1000000));

print("accel", s:setEventRate(5, 250000));
print("gyro", s:setEventRate(4, 250000));
print("magnetic", s:setEventRate(6, 250000));


svalue = {};
for i = 1,100 do
   unix.usleep(100000);
   e = s:getEvents();
   if (e) then
      print("sensor num events", #e);
      for i = 1,#e do
--	 print(" event", i);
--	 table.foreach(e[i], print);
	 svalue[e[i].sensor] = {unpack(e[i])};
      end
   end
end

for i = 1,#slist do
   s:disableSensor(i)
end

for i = 1,#svalue do
   print("svalue", i)
   table.foreach(svalue[i], print);
end