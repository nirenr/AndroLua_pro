package android.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.RotateAnimation;
import android.webkit.WebView;

import java.util.Timer;
import java.util.TimerTask;


/**
 * 自定义的布局，用来管理三个子控件，其中一个是下拉头，一个是包含内容的pullableView（可以是实现Pullable接口的的任何View），
 * 还有一个上拉头，更多详解见博客http://blog.csdn.net/zhongkejingwang/article/details/38868463
 *
 * @author 陈靖
 * @mod nirenr
 * 修改适合在androlua使用，懒得注释了
 */
@SuppressWarnings("deprecation")
public class PullingLayout extends RelativeLayout {
    public static final String TAG = "PullToRefreshLayout";
    // 初始状态
    public static final int INIT = 0;
    // 释放刷新
    public static final int RELEASE_TO_REFRESH = 1;
    // 正在刷新
    public static final int REFRESHING = 2;
    // 释放加载
    public static final int RELEASE_TO_LOAD = 3;
    // 正在加载
    public static final int LOADING = 4;
    // 操作完毕
    public static final int DONE = 5;
    // 刷新成功
    public static final int SUCCEED = 0;
    // 刷新失败
    public static final int FAIL = 1;
    // 没有内容
    public static final int NOTHING = 2;
    // 下拉的距离。注意：pullDownY和pullUpY不可能同时不为0
    public float pullDownY = 0;
    // 回滚速度
    public float MOVE_SPEED = 8;
    // 当前状态
    private int state = INIT;
    // 刷新回调接口
    private OnRefreshListener mRefreshListener;
    // 按下Y坐标，上一个事件点Y坐标
    private float downY, lastY;
    // 上拉的距离
    private float pullUpY = 0;
    // 释放刷新的距离
    private float refreshDist = 200;
    // 释放加载的距离
    private float loadmoreDist = 200;
    private MyTimer timer;
    // 第一次执行布局
    private boolean isLayout = false;
    // 在刷新过程中滑动操作
    private boolean isTouch = false;
    // 手指滑动距离与下拉头的滑动距离比，中间会随正切函数变化
    private float radio = 2;

    // 下拉箭头的转180°动画
    private RotateAnimation rotateAnimation;
    // 均匀旋转动画
    private RotateAnimation refreshingAnimation;

    // 下拉头
    private HeadView refreshView;
    // 下拉的箭头
    private View pullView;
    // 正在刷新的图标
    private View refreshingView;
    // 刷新结果图标
    private ImageView refreshStateImageView;
    // 刷新结果：成功或失败
    private TextView refreshStateTextView;

    // 上拉头
    private FootView loadmoreView;
    // 上拉的箭头
    private View pullUpView;
    // 正在加载的图标
    private View loadingView;
    // 加载结果图标
    private ImageView loadStateImageView;
    // 加载结果：成功或失败
    private TextView loadStateTextView;
    private int mStateColor;
    // 实现了Pullable接口的View
    private FrameLayout pullableLayout;
    private View pullableView;
    // 过滤多点触碰
    private int mEvents;
    // 这两个变量用来控制pull的方向，如果不加控制，当情况满足可上拉又可下拉时没法下拉
    private boolean canPullDown = true;
    private boolean canPullUp = true;
    private Context mContext;
    private boolean mPullUp;
    private boolean mPullDown;
    private LayoutInflater mInflater;
    private OnLoadMoreListener mLoadMoreListener;
    private PullingLayout.OnPullUpListener mPullUpListener;
    private PullingLayout.OnPullDownListener mPullDownListener;
    private DisplayMetrics dm;
    private int mForegroundColor;
    private int mBackgroundColor;
    private String refresh_succeed = "刷新成功";
    private String refresh_nothing = "暂无更新";
    private String refresh_fail = "刷新失败";
    private String load_succeed = "加载成功";
    private String load_nothing = "没有更多内容";
    private String load_fail = "加载失败";
    private String pullup_to_load = "上拉加载更多";
    private String release_to_refresh = "释放立即刷新";
    private String refreshing = "正在刷新...";
    private String release_to_load = "释放立即加载";
    private String pull_to_refresh = "下拉刷新";
    private String loading = "正在加载...";
    /**
     * 执行自动回滚的handler
     */
    Handler updateHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            // 回弹速度随下拉距离moveDeltaY增大而增大
            MOVE_SPEED = (float) (8 + 5 * Math.tan(Math.PI / 2
                    / getMeasuredHeight() * (pullDownY + Math.abs(pullUpY))));
            if (!isTouch) {
                // 正在刷新，且没有往上推的话则悬停，显示"正在刷新..."
                if (state == REFRESHING && pullDownY <= refreshDist) {
                    pullDownY = refreshDist;
                    timer.cancel();
                } else if (state == LOADING && -pullUpY <= loadmoreDist) {
                    pullUpY = -loadmoreDist;
                    timer.cancel();
                }

            }
            if (pullDownY > 0)
                pullDownY -= MOVE_SPEED;
            else if (pullUpY < 0)
                pullUpY += MOVE_SPEED;
            if (pullDownY < 0) {
                // 已完成回弹
                pullDownY = 0;
                pullView.clearAnimation();
                // 隐藏下拉头时有可能还在刷新，只有当前状态不是正在刷新时才改变状态
                if (state != REFRESHING && state != LOADING)
                    changeState(INIT);
                timer.cancel();
                requestLayout();
            }
            if (pullUpY > 0) {
                // 已完成回弹
                pullUpY = 0;
                pullUpView.clearAnimation();
                // 隐藏上拉头时有可能还在刷新，只有当前状态不是正在刷新时才改变状态
                if (state != REFRESHING && state != LOADING)
                    changeState(INIT);
                timer.cancel();
                requestLayout();
            }
            // 刷新布局,会自动调用onLayout
            requestLayout();
            // 没有拖拉或者回弹完成
            if (pullDownY + Math.abs(pullUpY) == 0)
                timer.cancel();
        }

    };

    public PullingLayout(Context context) {
        super(context);
        initView(context);
    }

    public PullingLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView(context);
    }

    public PullingLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initView(context);
    }

    public String getRefreshSucceed() {
        return refresh_succeed;
    }

    public void setRefreshSucceed(String refresh_succeed) {
        this.refresh_succeed = refresh_succeed;
    }

    public String getRefreshNothing() {
        return refresh_nothing;
    }

    public void setRefreshNothing(String refresh_nothing) {
        this.refresh_nothing = refresh_nothing;
    }

    public String getRefreshFail() {
        return refresh_fail;
    }

    public void setRefreshFail(String refresh_fail) {
        this.refresh_fail = refresh_fail;
    }

    public String getLoadSucceed() {
        return load_succeed;
    }

    public void setLoadSucceed(String load_succeed) {
        this.load_succeed = load_succeed;
    }

    public String getLoadNothing() {
        return load_nothing;
    }

    public void setLoadNothing(String load_nothing) {
        this.load_nothing = load_nothing;
    }

    public String getLoadFail() {
        return load_fail;
    }

    public void setLoadFail(String load_fail) {
        this.load_fail = load_fail;
    }

    public String getPullupToLoad() {
        return pullup_to_load;
    }

    public void setPullupToLoad(String pullup_to_load) {
        this.pullup_to_load = pullup_to_load;
    }

    public String getReleaseToRefresh() {
        return release_to_refresh;
    }

    public void setReleaseToRefresh(String release_to_refresh) {
        this.release_to_refresh = release_to_refresh;
    }

    public String getRefreshing() {
        return refreshing;
    }

    public void setRefreshing(String refreshing) {
        this.refreshing = refreshing;
    }

    public String getReleaseToLoad() {
        return release_to_load;
    }

    public void setReleaseToLoad(String release_to_load) {
        this.release_to_load = release_to_load;
    }

    /**
     * 完成刷新操作，显示刷新结果。注意：刷新完成后一定要调用这个方法
     */

    public String getPulldownToRefresh() {
        return pull_to_refresh;
    }

    public void setPulldownToRefresh(String pull_to_refresh) {
        this.pull_to_refresh = pull_to_refresh;
    }

    public String getLoading() {
        return loading;
    }

    public void setLoading(String loading) {
        this.loading = loading;
    }

    public void setOnRefreshListener(OnRefreshListener listener) {
        mRefreshListener = listener;
    }

    public void setOnLoadMoreListener(OnLoadMoreListener listener) {
        mLoadMoreListener = listener;
    }

    public void setOnPullUpListener(OnPullUpListener listener) {
        mPullUpListener = listener;
    }

    public void setOnPullDownListener(OnPullDownListener listener) {
        mPullDownListener = listener;
    }

    private int dp(float n) {

        return (int) TypedValue.applyDimension(1, n, dm);
    }

    public void setStateColor(int color) {
        mStateColor = color;
    }

    private void initView(Context context) {
        dm = context.getResources().getDisplayMetrics();

        mContext = context;
        TypedArray array = mContext.getTheme().obtainStyledAttributes(new int[]{
                android.R.attr.colorForeground,
                android.R.attr.colorBackground,
        });
        mForegroundColor = array.getColor(0, 0xFFFFFFFF);
        mBackgroundColor = array.getColor(1, 0xFF000000);
        array.recycle();
        setStateColor(mForegroundColor);

        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        refreshView = new HeadView(mContext);
        super.addView(refreshView, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        pullableLayout = new FrameLayout(mContext);
        super.addView(pullableLayout, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        loadmoreView = new FootView(mContext);
        super.addView(loadmoreView, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        pullableLayout = (FrameLayout) getChildAt(1);
        initView();

        timer = new MyTimer(updateHandler);
        rotateAnimation = new RotateAnimation(0, 180, Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
        rotateAnimation.setDuration(100);
        rotateAnimation.setRepeatCount(0);
        rotateAnimation.setFillAfter(true);

        refreshingAnimation = new RotateAnimation(0, 360, Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
        refreshingAnimation.setDuration(1500);
        refreshingAnimation.setRepeatCount(-1);
        refreshingAnimation.setFillAfter(true);

        // 添加匀速转动动画
        LinearInterpolator lir = new LinearInterpolator();
        rotateAnimation.setInterpolator(lir);
        refreshingAnimation.setInterpolator(lir);
    }

    private void hide() {
        timer.schedule(5);
    }

    /**
     * @param refreshResult PullToRefreshLayout.SUCCEED代表成功，PullToRefreshLayout.FAIL代表失败
     */
    public void refreshFinish(int refreshResult) {
        if (state != REFRESHING)
            return;
        refreshingView.clearAnimation();
        refreshingView.setVisibility(View.GONE);
        switch (refreshResult) {
            case SUCCEED:
                // 刷新成功
                refreshStateImageView.setVisibility(View.VISIBLE);
                refreshStateTextView.setText(refresh_succeed);
                refreshStateImageView
                        .setBackgroundDrawable(new SucceedDrawable());
                break;
            case NOTHING:
                // 没有更新
                refreshStateImageView.setVisibility(View.VISIBLE);
                refreshStateTextView.setText(refresh_nothing);
                refreshStateImageView
                        .setBackgroundDrawable(new FailDrawable());
                break;
            case FAIL:
            default:
                // 刷新失败
                refreshStateImageView.setVisibility(View.VISIBLE);
                refreshStateTextView.setText(refresh_fail);
                refreshStateImageView
                        .setBackgroundDrawable(new FailDrawable());
                break;
        }
        if (pullDownY > 0) {
            // 刷新结果停留1秒
            new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    changeState(DONE);
                    hide();
                }
            }.sendEmptyMessageDelayed(0, 1000);
        } else {
            changeState(DONE);
            hide();
        }
    }

    /**
     * 加载完毕，显示加载结果。注意：加载完成后一定要调用这个方法
     *
     * @param refreshResult PullToRefreshLayout.SUCCEED代表成功，PullToRefreshLayout.FAIL代表失败
     */
    public void loadmoreFinish(int refreshResult) {
        if (state != LOADING)
            return;
        loadingView.clearAnimation();
        loadingView.setVisibility(View.GONE);
        switch (refreshResult) {
            case SUCCEED:
                // 加载成功
                loadStateImageView.setVisibility(View.VISIBLE);
                loadStateTextView.setText(load_succeed);
                loadStateImageView.setBackgroundDrawable(new SucceedDrawable());
                break;
            case NOTHING:
                // 没有更多内容
                loadStateImageView.setVisibility(View.VISIBLE);
                loadStateTextView.setText(load_nothing);
                loadStateImageView.setBackgroundDrawable(new FailDrawable());
                break;
            case FAIL:
            default:
                // 加载失败
                loadStateImageView.setVisibility(View.VISIBLE);
                loadStateTextView.setText(load_fail);
                loadStateImageView.setBackgroundDrawable(new FailDrawable());
                break;
        }
        if (pullUpY < 0) {
            // 刷新结果停留1秒
            new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    changeState(DONE);
                    hide();
                }
            }.sendEmptyMessageDelayed(0, 1000);
        } else {
            changeState(DONE);
            hide();
        }
    }

    private void changeState(int to) {
        state = to;
        switch (state) {
            case INIT:
                // 下拉布局初始状态
                refreshStateImageView.setVisibility(View.GONE);
                refreshStateTextView.setText(pull_to_refresh);
                pullView.clearAnimation();
                pullView.setVisibility(View.VISIBLE);
                // 上拉布局初始状态
                loadStateImageView.setVisibility(View.GONE);
                loadStateTextView.setText(pullup_to_load);
                pullUpView.clearAnimation();
                pullUpView.setVisibility(View.VISIBLE);
                break;
            case RELEASE_TO_REFRESH:
                // 释放刷新状态
                refreshStateTextView.setText(release_to_refresh);
                pullView.startAnimation(rotateAnimation);
                break;
            case REFRESHING:
                // 正在刷新状态
                pullView.clearAnimation();
                refreshingView.setVisibility(View.VISIBLE);
                pullView.setVisibility(View.INVISIBLE);
                //refreshingView.startAnimation(refreshingAnimation);
                refreshStateTextView.setText(refreshing);
                break;
            case RELEASE_TO_LOAD:
                // 释放加载状态
                loadStateTextView.setText(release_to_load);
                pullUpView.startAnimation(rotateAnimation);
                break;
            case LOADING:
                // 正在加载状态
                pullUpView.clearAnimation();
                loadingView.setVisibility(View.VISIBLE);
                pullUpView.setVisibility(View.INVISIBLE);
                //loadingView.startAnimation(refreshingAnimation);
                loadStateTextView.setText(loading);
                break;
            case DONE:
                // 刷新或加载完毕，啥都不做
                break;
        }
    }

    /**
     * 不限制上拉或下拉
     */
    private void releasePull() {
        canPullDown = true;
        canPullUp = true;
    }

    /*
     * （非 Javadoc）由父控件决定是否分发事件，防止事件冲突
     *
     * @see android.view.ViewGroup#dispatchTouchEvent(android.view.MotionEvent)
     */
    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        switch (ev.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                downY = ev.getY();
                lastY = downY;
                timer.cancel();
                mEvents = 0;
                releasePull();
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
            case MotionEvent.ACTION_POINTER_UP:
                // 过滤多点触碰
                mEvents = -1;
                break;
            case MotionEvent.ACTION_MOVE:
                if (mEvents == 0) {
                    if (pullDownY > 0
                            || (canPullDown()
                            && canPullDown && state != LOADING)) {
                        // 可以下拉，正在加载时不能下拉
                        // 对实际滑动距离做缩小，造成用力拉的感觉
                        pullDownY = pullDownY + (ev.getY() - lastY) / radio;
                        if (pullDownY < 0) {
                            pullDownY = 0;
                            canPullDown = false;
                            canPullUp = true;
                        }
                        if (pullDownY > getMeasuredHeight())
                            pullDownY = getMeasuredHeight();
                        if (state == REFRESHING) {
                            // 正在刷新的时候触摸移动
                            isTouch = true;
                        }
                    } else if (pullUpY < 0
                            || (canPullUp() && canPullUp && state != REFRESHING)) {
                        // 可以上拉，正在刷新时不能上拉
                        pullUpY = pullUpY + (ev.getY() - lastY) / radio;
                        if (pullUpY > 0) {
                            pullUpY = 0;
                            canPullDown = true;
                            canPullUp = false;
                        }
                        if (pullUpY < -getMeasuredHeight())
                            pullUpY = -getMeasuredHeight();
                        if (state == LOADING) {
                            // 正在加载的时候触摸移动
                            isTouch = true;
                        }
                    } else
                        releasePull();
                } else
                    mEvents = 0;
                lastY = ev.getY();
                // 根据下拉距离改变比例
                radio = (float) (2 + 2 * Math.tan(Math.PI / 2 / getMeasuredHeight()
                        * (pullDownY + Math.abs(pullUpY))));
                if (pullDownY > 0 || pullUpY < 0)
                    requestLayout();
                if (pullDownY > 0) {
                    if (pullDownY <= refreshDist
                            && (state == RELEASE_TO_REFRESH || state == DONE)) {
                        // 如果下拉距离没达到刷新的距离且当前状态是释放刷新，改变状态为下拉刷新
                        changeState(INIT);
                    }
                    if (pullDownY >= refreshDist && state == INIT) {
                        // 如果下拉距离达到刷新的距离且当前状态是初始状态刷新，改变状态为释放刷新
                        changeState(RELEASE_TO_REFRESH);
                    }
                } else if (pullUpY < 0) {
                    // 下面是判断上拉加载的，同上，注意pullUpY是负值
                    if (-pullUpY <= loadmoreDist
                            && (state == RELEASE_TO_LOAD || state == DONE)) {
                        changeState(INIT);
                    }
                    // 上拉操作
                    if (-pullUpY >= loadmoreDist && state == INIT) {
                        changeState(RELEASE_TO_LOAD);
                    }

                }
                // 因为刷新和加载操作不能同时进行，所以pullDownY和pullUpY不会同时不为0，因此这里用(pullDownY +
                // Math.abs(pullUpY))就可以不对当前状态作区分了
                if ((pullDownY + Math.abs(pullUpY)) > 8) {
                    // 防止下拉过程中误触发长按事件和点击事件
                    ev.setAction(MotionEvent.ACTION_CANCEL);
                }
                break;
            case MotionEvent.ACTION_UP:
                if (pullDownY > refreshDist || -pullUpY > loadmoreDist) {
                    isTouch = false;
                }
                if (state == RELEASE_TO_REFRESH) {
                    changeState(REFRESHING);
                    // 刷新操作
                    if (mRefreshListener != null)
                        mRefreshListener.onRefresh(this);
                } else if (state == RELEASE_TO_LOAD) {
                    changeState(LOADING);
                    // 加载操作
                    if (mLoadMoreListener != null)
                        mLoadMoreListener.onLoadMore(this);
                }
                hide();
                break;
            default:
                break;
        }
        // 事件分发交给父类
        super.dispatchTouchEvent(ev);
        return true;
    }

    public void setPullUpEnabled(boolean canPullUp) {

        mPullUp = canPullUp;
    }

    private boolean canPullUp() {

        if (!mPullUp || pullableView == null)
            return false;
        if (mPullUpListener != null)
            return mPullUpListener.onPullUp(pullableView);

        if (pullableView instanceof ListView)
            return listViewCanPullUp((ListView) pullableView);
        if (pullableView instanceof GridView)
            return gridViewCanPullUp((GridView) pullableView);
        if (pullableView instanceof ExpandableListView)
            return expandableListViewCanPullUp((ExpandableListView) pullableView);
        if (pullableView instanceof ScrollView)
            return scrollViewCanPullUp((ScrollView) pullableView);
        if (pullableView instanceof WebView)
            return webViewCanPullUp((WebView) pullableView);

        return true;
    }

    private boolean expandableListViewCanPullUp(ExpandableListView view) {

        if (view.getCount() == 0) {
            // 没有item的时候也可以上拉加载
            return true;
        } else if (view.getLastVisiblePosition() == (view.getCount() - 1)) {
            // 滑到底部了
            if (view.getChildAt(view.getLastVisiblePosition() - view.getFirstVisiblePosition()) != null
                    && view.getChildAt(
                    view.getLastVisiblePosition()
                            - view.getFirstVisiblePosition()).getBottom() <= view.getMeasuredHeight())
                return true;
        }
        return false;
    }

    private boolean gridViewCanPullUp(GridView view) {

        if (view.getCount() == 0) {
            // 没有item的时候也可以上拉加载
            return true;
        } else if (view.getLastVisiblePosition() == (view.getCount() - 1)) {
            // 滑到底部了
            if (view.getChildAt(view.getLastVisiblePosition() - view.getFirstVisiblePosition()) != null
                    && view.getChildAt(
                    view.getLastVisiblePosition()
                            - view.getFirstVisiblePosition()).getBottom() <= view.getMeasuredHeight())
                return true;
        }
        return false;

    }

    private boolean scrollViewCanPullUp(ScrollView view) {

        if (view.getScrollY() >= (view.getChildAt(0).getHeight() - view.getMeasuredHeight()))
            return true;
        else
            return false;
    }

    private boolean webViewCanPullUp(WebView view) {

        if (view.getScrollY() >= view.getContentHeight() * view.getScale()
                - view.getMeasuredHeight())
            return true;
        else
            return false;

    }

    private boolean listViewCanPullUp(ListView view) {

        if (view.getCount() == 0) {
            // 没有item的时候也可以上拉加载
            return true;
        } else if (view.getLastVisiblePosition() == (view.getCount() - 1)) {
            // 滑到底部了
            if (view.getChildAt(view.getLastVisiblePosition() - view.getFirstVisiblePosition()) != null
                    && view.getChildAt(
                    view.getLastVisiblePosition()
                            - view.getFirstVisiblePosition()).getBottom() <= view.getMeasuredHeight())
                return true;
        }
        return false;
    }

    public void setPullDownEnabled(boolean canPullDown) {

        mPullDown = canPullDown;
    }

    private boolean canPullDown() {

        if (mPullDown == false || pullableView == null)
            return false;
        if (mPullDownListener != null)
            return mPullDownListener.onPullDown(pullableView);

        if (pullableView instanceof AbsListView)
            return absListViewCanPullDown((AbsListView) pullableView);
        if (pullableView instanceof ScrollView)
            return viewCanPullDown((ScrollView) pullableView);
        if (pullableView instanceof WebView)
            return viewCanPullDown((WebView) pullableView);

        return true;
    }

    private boolean viewCanPullDown(View view) {

        if (view.getScrollY() == 0)
            return true;
        else
            return false;

    }

    private boolean absListViewCanPullDown(AbsListView view) {

        if (view.getCount() == 0) {
            // 没有item的时候也可以下拉刷新
            return true;
        } else if (view.getFirstVisiblePosition() == 0
                && view.getChildAt(0).getTop() >= 0) {
            // 滑到ListView的顶部了
            return true;
        } else
            return false;

    }

    /**
     * 自动刷新
     */
    public void autoRefresh() {
        AutoRefreshAndLoadTask task = new AutoRefreshAndLoadTask();
        task.execute(20);
    }

    /**
     * 自动加载
     */
    public void autoLoad() {
        pullUpY = -loadmoreDist;
        requestLayout();
        changeState(LOADING);
        // 加载操作
        if (mLoadMoreListener != null)
            mLoadMoreListener.onLoadMore(this);
    }

    private void initView() {
        // 初始化下拉布局
        pullView = refreshView.getPullView();
        refreshStateTextView = refreshView.getStateText();
        refreshingView = refreshView.getLoadingView();
        refreshStateImageView = refreshView.getStateView();
        // 初始化上拉布局
        pullUpView = loadmoreView.getPullView();
        loadStateTextView = loadmoreView.getStateText();
        loadingView = loadmoreView.getLoadingView();
        loadStateImageView = loadmoreView.getStateView();
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        if (!isLayout) {
            // 这里是第一次进来的时候做一些初始化
            isLayout = true;
            refreshDist = ((ViewGroup) refreshView).getChildAt(0)
                    .getMeasuredHeight();
            loadmoreDist = ((ViewGroup) loadmoreView).getChildAt(0)
                    .getMeasuredHeight();
            loadStateTextView.setTextColor(mStateColor);
            refreshStateTextView.setTextColor(mStateColor);
        }
        // 改变子控件的布局，这里直接用(pullDownY + pullUpY)作为偏移量，这样就可以不对当前状态作区分
        refreshView.layout(0,
                (int) (pullDownY + pullUpY) - refreshView.getMeasuredHeight(),
                refreshView.getMeasuredWidth(), (int) (pullDownY + pullUpY));
        pullableLayout.layout(0, (int) (pullDownY + pullUpY),
                pullableLayout.getMeasuredWidth(), (int) (pullDownY + pullUpY)
                        + pullableLayout.getMeasuredHeight());
        loadmoreView.layout(0,
                (int) (pullDownY + pullUpY) + pullableLayout.getMeasuredHeight(),
                loadmoreView.getMeasuredWidth(),
                (int) (pullDownY + pullUpY) + pullableLayout.getMeasuredHeight()
                        + loadmoreView.getMeasuredHeight());
    }

    @Override
    public void addView(View child, ViewGroup.LayoutParams params) {

        child.setLayoutParams(new FrameLayout.LayoutParams(params.width, params.height));
        pullableView = child;
        pullableLayout.addView(child);
    }

    @Override
    public void addView(View child) {

        pullableView = child;
        pullableLayout.addView(child);
    }

    /**
     * 刷新回调接口
     *
     * @author chenjing
     */
    public interface OnRefreshListener {
        /**
         * 刷新操作
         */
        void onRefresh(PullingLayout pullToRefreshLayout);
    }

    /**
     * 加载回调接口
     *
     * @author chenjing
     */
    public interface OnLoadMoreListener {

        /**
         * 加载操作
         */
        void onLoadMore(PullingLayout pullToRefreshLayout);
    }

    public interface OnPullUpListener {
        public boolean onPullUp(View view);
    }

    public interface OnPullDownListener {
        public boolean onPullDown(View view);
    }

    /**
     * @author chenjing 自动模拟手指滑动的task
     */
    private class AutoRefreshAndLoadTask extends
            AsyncTask<Integer, Float, String> {

        @Override
        protected String doInBackground(Integer... params) {
            while (pullDownY < 4 / 3 * refreshDist) {
                pullDownY += MOVE_SPEED;
                publishProgress(pullDownY);
                try {
                    Thread.sleep(params[0]);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            return null;
        }

        @Override
        protected void onPostExecute(String result) {
            changeState(REFRESHING);
            // 刷新操作
            if (mRefreshListener != null)
                mRefreshListener.onRefresh(PullingLayout.this);
            hide();
        }

        @Override
        protected void onProgressUpdate(Float... values) {
            if (pullDownY > refreshDist)
                changeState(RELEASE_TO_REFRESH);
            requestLayout();
        }

    }

    class MyTimer {
        private Handler handler;
        private Timer timer;
        private MyTask mTask;

        public MyTimer(Handler handler) {
            this.handler = handler;
            timer = new Timer();
        }

        public void schedule(long period) {
            if (mTask != null) {
                mTask.cancel();
                mTask = null;
            }
            mTask = new MyTask(handler);
            timer.schedule(mTask, 0, period);
        }

        public void cancel() {
            if (mTask != null) {
                mTask.cancel();
                mTask = null;
            }
        }

        class MyTask extends TimerTask {
            private Handler handler;

            public MyTask(Handler handler) {
                this.handler = handler;
            }

            @Override
            public void run() {
                handler.obtainMessage().sendToTarget();
            }

        }
    }

    public class HeadView extends RelativeLayout {

        private ImageView mLoadingView;
        private TextView mStateText;
        private ImageView mStateView;
        private ImageView mPullView;

        public HeadView(Context context) {
            super(context);
            initView(context);
        }


        public HeadView(Context context, AttributeSet attrs) {
            super(context, attrs);
            initView(context);
        }

        public HeadView(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
            initView(context);
        }

        public ImageView getStateView() {
            return mStateView;
        }

        public TextView getStateText() {
            return mStateText;
        }

        public ImageView getLoadingView() {
            return mLoadingView;
        }

        public ImageView getPullView() {
            return mPullView;
        }

        private void initView(Context context) {

            int w = dp(30);

            RelativeLayout layout = new RelativeLayout(context);
            layout.setPadding(0, dp(20), 0, dp(20));
            RelativeLayout.LayoutParams lp = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
            lp.addRule(12);
            addView(layout, lp);

            RelativeLayout layout2 = new RelativeLayout(context);
            RelativeLayout.LayoutParams lp2 = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
            lp2.addRule(13);
            layout.addView(layout2, lp2);

            mPullView = new ImageView(context);
            mPullView.setBackgroundDrawable(new PullDownDrawable());
            RelativeLayout.LayoutParams lp3 = new LayoutParams(w, w);
            lp3.setMargins(dp(60), 0, 0, 0);
            lp3.addRule(15);
            layout2.addView(mPullView, lp3);

            mLoadingView = new ImageView(context);
            mLoadingView.setVisibility(GONE);
            mLoadingView.setBackgroundDrawable(new LoadingDrawable());
            RelativeLayout.LayoutParams lp4 = new LayoutParams(w, w);
            lp4.setMargins(dp(60), 0, 0, 0);
            lp4.addRule(15);
            layout2.addView(mLoadingView, lp4);

            mStateText = new TextView(context);
            mStateText.setText(pull_to_refresh);
            mStateText.setTextSize(16);
            RelativeLayout.LayoutParams lp5 = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
            lp5.addRule(13);
            layout2.addView(mStateText, lp5);

            mStateView = new ImageView(context);
            mStateView.setVisibility(GONE);
            RelativeLayout.LayoutParams lp6 = new LayoutParams(w, w);
            lp6.setMargins(dp(60), 0, 0, 0);
            lp6.addRule(15);
            layout2.addView(mStateView, lp6);
        }

    }

    public class FootView extends RelativeLayout {

        private ImageView mLoadingView;
        private TextView mStateText;
        private ImageView mStateView;
        private ImageView mPullView;

        public FootView(Context context) {
            super(context);
            initView(context);
        }

        public FootView(Context context, AttributeSet attrs) {
            super(context, attrs);
            initView(context);
        }

        public FootView(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
            initView(context);
        }

        public ImageView getStateView() {
            return mStateView;
        }

        public TextView getStateText() {
            return mStateText;
        }

        public ImageView getLoadingView() {
            return mLoadingView;
        }

        public ImageView getPullView() {
            return mPullView;
        }

        private void initView(Context context) {

            int w = dp(30);
            RelativeLayout layout = new RelativeLayout(context);
            layout.setPadding(0, dp(20), 0, dp(20));
            RelativeLayout.LayoutParams lp = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
            lp.addRule(10);
            addView(layout, lp);

            RelativeLayout layout2 = new RelativeLayout(context);
            RelativeLayout.LayoutParams lp2 = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
            lp2.addRule(13);
            layout.addView(layout2, lp2);

            mPullView = new ImageView(context);
            mPullView.setBackgroundDrawable(new PullUpDrawable());
            RelativeLayout.LayoutParams lp3 = new LayoutParams(w, w);
            lp3.setMargins(dp(60), 0, 0, 0);
            lp3.addRule(15);
            layout2.addView(mPullView, lp3);

            mLoadingView = new ImageView(context);
            mLoadingView.setVisibility(GONE);
            mLoadingView.setBackgroundDrawable(new LoadingDrawable());
            RelativeLayout.LayoutParams lp4 = new LayoutParams(w, w);
            lp4.setMargins(dp(60), 0, 0, 0);
            lp4.addRule(15);
            layout2.addView(mLoadingView, lp4);

            mStateText = new TextView(context);
            mStateText.setText(pullup_to_load);
            mStateText.setTextSize(16);
            RelativeLayout.LayoutParams lp5 = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
            lp5.addRule(13);
            layout2.addView(mStateText, lp5);

            mStateView = new ImageView(context);
            mStateView.setVisibility(GONE);
            RelativeLayout.LayoutParams lp6 = new LayoutParams(w, w);
            lp6.setMargins(dp(60), 0, 0, 0);
            lp6.addRule(15);
            layout2.addView(mStateView, lp6);

        }

    }

    private class PullDownDrawable extends Drawable {

        private Paint p;

        PullDownDrawable() {
            p = new Paint();
            p.setStyle(Paint.Style.STROKE);
            p.setAntiAlias(true);
            p.setStrokeWidth(dp(2));
            //p.setShadowLayer(dp(2),dp(2),dp(2), Color.GRAY);
        }

        @Override
        public void draw(Canvas c) {

            p.setColor(mStateColor);
            Rect b = getBounds();
            int r = (int) ((float) Math.min(b.right, b.bottom) * 0.35);
            c.drawCircle(b.right / 2, b.bottom / 2, r, p);
            Path path = new Path();
            path.moveTo(b.right * 0.5f, b.bottom * 0.25f);
            path.lineTo(b.right * 0.5f, b.bottom * 0.75f);
            path.moveTo(b.right * 0.25f, b.bottom * 0.5f);
            path.lineTo(b.right * 0.5f, b.bottom * 0.75f);
            path.lineTo(b.right * 0.75f, b.bottom * 0.5f);
            c.drawPath(path, p);
            //c.drawLine(b.right / 2, b.bottom / 4, b.right / 2, b.bottom / 4 * 3, p);
            //c.drawLine(b.right / 4, b.bottom / 2, b.right / 2, b.bottom / 4 * 3, p);
            //c.drawLine(b.right / 4 * 3, b.bottom / 2, b.right / 2, b.bottom / 4 * 3, p);
        }

        @Override
        public void setAlpha(int p1) {

            p.setAlpha(p1);
        }

        @Override
        public void setColorFilter(ColorFilter p1) {

            p.setColorFilter(p1);
        }

        @Override
        public int getOpacity() {

            return PixelFormat.UNKNOWN;
        }
    }

    private class PullUpDrawable extends Drawable {

        private Paint p;

        PullUpDrawable() {
            p = new Paint();
            p.setStyle(Paint.Style.STROKE);
            p.setAntiAlias(true);
            p.setStrokeWidth(dp(2));
            //p.setShadowLayer(dp(2),dp(2),dp(2), Color.GRAY);
        }

        @Override
        public void draw(Canvas c) {

            p.setColor(mStateColor);
            Rect b = getBounds();
            int r = (int) ((float) Math.min(b.right, b.bottom) * 0.35);
            c.drawCircle(b.right / 2, b.bottom / 2, r, p);
            Path path = new Path();
            path.moveTo(b.right * 0.5f, b.bottom * 0.25f);
            path.lineTo(b.right * 0.5f, b.bottom * 0.75f);
            path.moveTo(b.right * 0.25f, b.bottom * 0.5f);
            path.lineTo(b.right * 0.5f, b.bottom * 0.25f);
            path.lineTo(b.right * 0.75f, b.bottom * 0.5f);
            c.drawPath(path, p);
            //c.drawLine(b.right / 2, b.bottom / 4, b.right / 2, b.bottom / 4 * 3, p);
            //c.drawLine(b.right / 4, b.bottom / 2, b.right / 2, b.bottom / 4, p);
            //c.drawLine(b.right / 4 * 3, b.bottom / 2, b.right / 2, b.bottom / 4, p);
        }

        @Override
        public void setAlpha(int p1) {

            p.setAlpha(p1);
        }

        @Override
        public void setColorFilter(ColorFilter p1) {

            p.setColorFilter(p1);
        }

        @Override
        public int getOpacity() {

            return PixelFormat.UNKNOWN;
        }
    }

    private class LoadingDrawable2 extends Drawable {

        private Paint p;
        private int n = 0;
        private int m = 0;
        private int sn = 6;
        private int sm = 2;

        LoadingDrawable2() {
            p = new Paint();
            p.setStyle(Paint.Style.STROKE);
            p.setAntiAlias(true);
            p.setStrokeWidth(dp(2));
            //p.setShadowLayer(dp(2),dp(2),dp(2), Color.GRAY);
        }

        @Override
        public void draw(Canvas c) {

            p.setColor(mStateColor);
            Rect b = getBounds();
            int r = (int) ((float) Math.min(b.right, b.bottom) * 0.35);
            RectF f = new RectF(r / 2, r / 2, r * 2.5f, r * 2.5f);
            if (n > 360) {
                sm += sn;
                sn = 0 - sn;
            } else if (n < 6) {
                sn = 6;
                sm = 2;
            }
            n += sn;
            m += sm;
            c.drawArc(f, m % 360, n, false, p);
            invalidateSelf();
            /*float f = (float) r * 1.5f;
            SweepGradient mShader = new SweepGradient(f, f, mBackgroundColor, mStateColor);
            p.setShader(mShader);
            DashPathEffect effects = new DashPathEffect(new float[]{r / 5, r / 6}, 1);
            p.setPathEffect(effects);
            c.drawCircle(b.right / 2, b.bottom / 2, r, p);*/
        }

        @Override
        public void setAlpha(int p1) {

            p.setAlpha(p1);
        }

        @Override
        public void setColorFilter(ColorFilter p1) {

            p.setColorFilter(p1);
        }

        @Override
        public int getOpacity() {

            return PixelFormat.UNKNOWN;
        }
    }


    private class SucceedDrawable2 extends Drawable {

        private Paint p;

        SucceedDrawable2() {
            p = new Paint();
            p.setStyle(Paint.Style.STROKE);
            p.setAntiAlias(true);
            p.setStrokeWidth(dp(2));
            //p.setShadowLayer(dp(2),dp(2),dp(2), Color.GRAY);
        }

        @Override
        public void draw(Canvas c) {

            p.setColor(mStateColor);
            Rect b = getBounds();
            int r = (int) ((float) Math.min(b.right, b.bottom) * 0.35);
            c.drawCircle(b.right / 2, b.bottom / 2, r, p);
            Path path = new Path();
            path.moveTo(b.right * 0.3f, b.bottom * 0.5f);
            path.lineTo(b.right * 0.45f, b.bottom * 0.7f);
            path.lineTo(b.right * 0.75f, b.bottom * 0.4f);
            c.drawPath(path, p);
            //c.drawLine(b.right * 0.3f, b.bottom / 2, b.right * 0.45f, b.bottom * 0.7f, p);
            //c.drawLine(b.right / 4 * 3, b.bottom * 0.4f, b.right * 0.45f, b.bottom * 0.7f, p);
        }

        @Override
        public void setAlpha(int p1) {

            p.setAlpha(p1);
        }

        @Override
        public void setColorFilter(ColorFilter p1) {

            p.setColorFilter(p1);
        }

        @Override
        public int getOpacity() {

            return PixelFormat.UNKNOWN;
        }
    }

    private class FailDrawable2 extends Drawable {

        private Paint p;

        FailDrawable2() {
            p = new Paint();
            p.setStyle(Paint.Style.STROKE);
            p.setAntiAlias(true);
            p.setStrokeWidth(dp(2));
            //p.setShadowLayer(dp(2),dp(2),dp(2), Color.GRAY);
        }

        @Override
        public void draw(Canvas c) {

            p.setColor(mStateColor);
            Rect b = getBounds();
            int r = (int) ((float) Math.min(b.right, b.bottom) * 0.35);
            c.drawCircle(b.right / 2, b.bottom / 2, r, p);
            c.drawLine(b.right / 2, b.bottom * 0.25f, b.right / 2, b.bottom * 0.65f, p);
            c.drawLine(b.right / 2, b.bottom * 0.7f, b.right / 2, b.bottom * 0.75f, p);
        }

        @Override
        public void setAlpha(int p1) {
            p.setAlpha(p1);
        }

        @Override
        public void setColorFilter(ColorFilter p1) {

            p.setColorFilter(p1);
        }

        @Override
        public int getOpacity() {

            return PixelFormat.UNKNOWN;
        }
    }

    private int n = 0;
    private int m = 0;
    private int x = 0;
    private int y = 0;
    private int sn = 6;
    private int sm = 2;

    private class LoadingDrawable extends Drawable {

        static final int STATE_LOADING = 0;
        static final int STATE_SUCCESS = 1;
        static final int STATE_FAIL = -1;
        private Paint p;
        private int mState;

        LoadingDrawable() {
            p = new Paint();
            p.setStyle(Paint.Style.STROKE);
            p.setAntiAlias(true);
            p.setStrokeWidth(dp(2));
        }

        void setState(int state) {
            mState = state;
        }

        void loading() {
            reset();
        }

        void succe() {
            mState = STATE_SUCCESS;
        }

        void fail() {
            mState = STATE_FAIL;
        }

        void reset() {
            mState = STATE_LOADING;
            sn = 6;
            sm = 2;
            n = 0;
            m = 0;
            x = 0;
            y = 0;
            invalidateSelf();
        }

        @Override
        public void draw(Canvas c) {

            p.setColor(mStateColor);
            Rect b = getBounds();
            int r = (int) ((float) Math.min(b.right, b.bottom));
            RectF f = new RectF(r * 0.15f, r * 0.15f, r * 0.85f, r * 0.85f);
            if (n >= 360 && mState == STATE_LOADING) {
                sm = 8;
                sn = 0 - 6;
            } else if (n <= 6) {
                sn = 6;
                sm = 2;
            }
            if (n < 360 || mState == STATE_LOADING) {
                if (mState == STATE_LOADING) {
                    n += sn;
                    m += sm;
                    m %= 360;
                } else {
                    n += sn * 2;
                    m += sm * 2;
                    m %= 360;
                }
            }
            c.drawArc(f, m, n, false, p);

            if (n >= 360) {
                sn = -6;
                sm = 8;

                if (mState == STATE_SUCCESS) {
                    Path path = new Path();
                    path.moveTo(b.right * 0.3f, b.bottom * 0.5f);
                    path.lineTo(b.right * 0.45f, b.bottom * 0.7f);
                    path.lineTo(b.right * 0.75f, b.bottom * 0.4f);
                    c.drawPath(path, p);
                } else if (mState == STATE_FAIL) {
                    c.drawLine(b.right / 2, b.bottom * 0.25f, b.right / 2, b.bottom * 0.65f, p);
                    c.drawLine(b.right / 2, b.bottom * 0.7f, b.right / 2, b.bottom * 0.75f, p);
                }
            }
            invalidateSelf();
        }

        @Override
        public void setAlpha(int p1) {

            p.setAlpha(p1);
        }

        @Override
        public void setColorFilter(ColorFilter p1) {

            p.setColorFilter(p1);
        }

        @Override
        public int getOpacity() {

            return PixelFormat.UNKNOWN;
        }
    }

    private class SucceedDrawable extends LoadingDrawable {

        SucceedDrawable() {
            super();
            succe();
        }

    }

    private class FailDrawable extends LoadingDrawable {

        FailDrawable() {
            super();
            fail();
        }
    }
}

