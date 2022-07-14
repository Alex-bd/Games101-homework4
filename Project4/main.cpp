#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

//cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
//{
//    // TODO: Implement de Casteljau's algorithm
//    return cv::Point2f();
//}

//bool recursive_bezier(const std::vector<cv::Point2f>& control_points, std::vector<cv::Point2f>& points1, std::vector<cv::Point2f>& points2, float t, cv::Mat& window) {
//    int size;
//    size = points1.size();
//
//    if (size == 1) {
//        window.at<cv::Vec3b>(points1[0].y, points1[0].x)[1] = 255;//设置该点的绿通道为255  b g r ，1为green通道
//        points2 = control_points;   //points2存放找到的该t时刻的唯一的点
//        points1.clear();//结束计算后记得初始化容器
//        return true;
//    }
//    for (int i = 0; i < size - 1; i++)
//        points2.push_back((1 - t) * points1[i] + t * points1[i+1]);   // bo1(t) = (1-t)b0 +tb1
//    points1.clear();
//    return false;
//}

// 实现对Bezier曲线的反走样
bool recursive_bezier(const std::vector<cv::Point2f>& control_points, std::vector<cv::Point2f>& points1, std::vector<cv::Point2f>& points2, float t, cv::Mat& window) {
    int size;
    size = points1.size();
    double ratio = 1;
    if (size == 1) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {//遍历9个像素
                if (points1[0].y + j > 700 || points1[0].y + j < 0 || points1[0].x + i > 700 || points1[0].x + i < 0)//不处理越界像素
                    continue;

                // 利用[1- （p和周围8个点的欧氏距离）/3] 计算得到一个 255像素的程度 比率-零点几的个数
                ratio = 1 - sqrt(2) * sqrt(pow(points1[0].y - int(points1[0].y + j) - 0.5, 2) + pow(points1[0].x - int(points1[0].x + i) - 0.5, 2)) / 3;//计算ratio
                // 显示的是 该点周围八邻域内的最大值，以达到实现反走样
                window.at<cv::Vec3b>(points1[0].y + j, points1[0].x + i)[1] = std::fmax(window.at<cv::Vec3b>(points1[0].y + j, points1[0].x + i)[1], 255 * ratio);//计算像素颜色
            }
        }
        points2 = control_points;
        points1.clear();
        return true;
    }
    for (int i = 0; i < size - 1; i++)
        points2.push_back((1 - t) * points1[i] + t * points1[i + 1]);  // bo1(t) = (1-t)b0 +tb1
    points1.clear();
    return false;
}




void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    int size = control_points.size();   // 获得控制点个数
    std::vector<cv::Point2f> points1 = control_points;   // 容器1
    std::vector<cv::Point2f> points2; // 容器2
    bool flag = true, bflag;
    for (double t = 0.0; t <= 1.0; t += 0.001) {
        bflag = false;
        while (!bflag) {
            if (flag) {
                bflag = recursive_bezier(control_points, points1, points2, t, window);
            }
            else {
                bflag = recursive_bezier(control_points, points2, points1, t, window);
            }
            flag = !flag;
        }
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points)     // 绘制控制点
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4)         //控制点到达4个
        {
            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }
        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }
return 0;
}
