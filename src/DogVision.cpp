#include "DogVision.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

// 无参构造, 只是打开摄像头
DogVision::DogVision() : cam("../config/stereo_camera_config.yaml") {

}

// 有参构造, 构造函数会读取配置文件，并根据文件内容设置参数值, 并传入控制运动的指针
DogVision::DogVision(const std::string& configFile, DogMotion* myPlan) : cam("../config/stereo_camera_config.yaml") {
    dogMotion = myPlan;
    std::ifstream config_file(configFile);
    std::string line;
    // read the config file
    while (std::getline(config_file, line)) {
        std::istringstream iss(line);
        std::string name;
        int value;
        char equal_sign;
        if (!(iss >> name >> equal_sign >> value) || equal_sign != '=') {
            throw std::runtime_error("Error parsing config file: " + line);
        }
        if (name == "cannyThreshold") {
            cannyThreshold = value;
        } else if (name == "cannyThresholdMax") {
            cannyThresholdMax = value;
        } else if (name == "houghThreshold") {
            houghThreshold = value;
        } else if (name == "houghMinLineLength") {
            houghMinLineLength = value;
        } else if (name == "houghMaxLineGap") {
            houghMaxLineGap = value;
        } else if (name == "yellowHLower") {
            yellowHLower = value;
        } else if (name == "yellowSLower") {
            yellowSLower = value;
        } else if (name == "yellowVLower") {
            yellowVLower = value;
        } else if (name == "yellowHUpper") {
            yellowHUpper = value;
        } else if (name == "yellowSUpper") {
            yellowSUpper = value;
        } else if (name == "yellowVUpper") {
            yellowVUpper = value;
        } else if (name == "lowerAngel") {
            lowerAngel = value;
        } else if (name == "upperAngel") {
            upperAngel = value;
        } else if (name == "dropNode1") {
            dogMotion->flag1 = value;
        } else if (name == "dropNode2") {
            dogMotion->flag2 = value;
        } else if (name == "dropNode3") {
            dogMotion->flag3 = value;
        } else if (name == "dropNode4") {
            dogMotion->flag4 = value;
        } else {
            throw std::runtime_error("Unknown parameter in config file: " + name);
        }
    }
}


// 这个函数用于处理图像，你需要根据你的具体需求来实现这个函数。
void DogVision::processImage(cv::Mat& src) {
    Mat hsv;
    cvtColor(src, hsv, COLOR_BGR2HSV);
    medianBlur(hsv, hsv, 5);
    Mat mask;

    // Use the config values for yellow color range
    Scalar yellow_lower = Scalar(yellowHLower, yellowSLower, yellowVLower);
    Scalar yellow_upper = Scalar(yellowHUpper, yellowSUpper, yellowVUpper);
    inRange(hsv, yellow_lower, yellow_upper, mask);
    Mat road;
    bitwise_and(src, src, road, mask);
    road.setTo(Scalar(255, 0, 255), mask);
    addWeighted(src, 1, road, 1, 0, src);
    mask(Rect(0, 0, mask.cols, mask.rows * 2 / 7)) = 0; // UP
    mask(Rect(0, mask.rows * 2 / 7, mask.cols / 7, mask.rows * 4 / 7)) = 0; // left
    mask(Rect(mask.cols * 6 / 7, mask.rows * 2 / 7, mask.cols / 7, mask.rows * 4 / 7)) = 0; // right
    mask(Rect(0, mask.rows * 6 / 7, mask.cols, mask.rows * 1 / 7 + 1)) = 0; // beneath
    Mat edges;
    Canny(mask, edges, cannyThreshold, cannyThresholdMax);
    vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI / 180, houghThreshold, houghMinLineLength, houghMaxLineGap);

// Filter and classify lines
    vector<Vec4i> left_lines, right_lines;
    for (auto line: lines) {
        double angle = abs(atan2(line[3] - line[1], line[2] - line[0]) * 180.0 / CV_PI);
        //cout << "angle: " << angle << endl;
        if ((angle >= lowerAngel) && (angle < upperAngel)) {
            if (line[2] < src.cols / 2) {
                left_lines.push_back(line);
            } else {
                right_lines.push_back(line);
            }
        }
    }

    // Draw the line of direction
    line(src, Point(src.cols / 2, 0), Point(src.cols / 2, src.rows), Scalar(0, 255, 0), 2);

// Check if we have both left and right lines
    if (left_lines.empty() || right_lines.empty()) {
        //cout << "Can't find two boundary" << endl;
        // Mat merge;
        // hconcat(mask, edges, merge);
        // imshow("Detected Lines", src);
        // imshow("mask edges", merge);
        //waitKey(1);
    } else {

// Calculate the average center point of all left and right lines
        Point left_center(0, 0), right_center(0, 0);
        for (auto line: left_lines) {
            left_center.x += (line[0] + line[2]) / 2;
            left_center.y += (line[1] + line[3]) / 2;
        }
        for (auto line: right_lines) {
            right_center.x += (line[0] + line[2]) / 2;
            right_center.y += (line[1] + line[3]) / 2;
        }
        left_center.x /= left_lines.size();
        left_center.y /= left_lines.size();
        right_center.x /= right_lines.size();
        right_center.y /= right_lines.size();

// Calculate and draw the center point
        Point center_point((left_center.x + right_center.x) / 2, (left_center.y + right_center.y) / 2);
        circle(src, center_point, 5, Scalar(0, 0, 255), -1);

// use motiontime to control:
        if (center_point.x < (src.cols / 2 - 20) && !dogMotion->yellowLeft && !dogMotion->yellowRight) { // delete isTime()
            cout << "Mid Correcting Right ! !" << endl;
            dogMotion->isVision = true;
            dogMotion->midShiftRight = true;
            sleep(1);
            dogMotion->midShiftRight = false;
            dogMotion->isVision = false;
        }
        if (center_point.x > (src.cols / 2 + 20) && !dogMotion->yellowLeft && !dogMotion->yellowRight) {  // delete isTime()
            cout << " Mid Correcting Left ! !" << endl;
            // dogMotion->isVision = true;
            dogMotion->isVision = true;
            dogMotion->midShiftLeft = true;
            sleep(1);
            dogMotion->midShiftLeft = false;
            dogMotion->isVision = false;
            // Mat merge;
            // hconcat(mask, edges, merge);
            // imshow("Detected Lines", src);
            // imshow("mask edges", merge);
            // imshow("menubur", temp);
            //waitKey(1);
        }
    }
}

void DogVision::detectYellow(cv::Mat& image)
{
    //预处理图像
    Mat src = image;
    int rows = image.rows;
    int cols = image.cols;
    int left = cols / 5;
    int right = cols * 4 / 5;
    Rect roi(left, 0, right - left, rows);
    image = image(roi);
    /*int top = rows / 4;
    int bottom = rows * 3 / 4;
    Rect roi(0, top, cols, bottom - top);
    image = image(roi);*/
    medianBlur(image, image, 5);

    // 创建一个掩膜，只显示黄色区域
    Mat hsv;
    cvtColor(image, hsv, COLOR_BGR2HSV);
    Scalar lowerYellow = Scalar(yellowHLower, yellowSLower, yellowVLower);
    Scalar upperYellow = Scalar(yellowHUpper, yellowSUpper, yellowVUpper);
    Mat yellowMask;
    inRange(hsv, lowerYellow, upperYellow, yellowMask);

    // 在原图截取黄色道路
    Mat road;
    bitwise_and(image, image, road, yellowMask);

    //显示图像
    //imshow("image", image);
    //imshow("road",road);
    //imshow("src", src);

    //判断越界
    string direction = "";
    int nonZeroCount = countNonZero(yellowMask);
    double currentRatio = 1.0 * nonZeroCount / (yellowMask.rows * yellowMask.cols);
    double normalRatio = 0.45;
    if (currentRatio < normalRatio)
    {
        Mat leftMask = yellowMask(Rect(0, 0, image.cols / 2, image.rows));
        Mat rightMask = yellowMask(Rect(image.cols / 2, 0, image.cols / 2, image.rows));
        int leftCount = countNonZero(leftMask);
        int rightCount = countNonZero(rightMask);

        // control with motiontime and condition
        if (leftCount < rightCount) // delete isTime()
        {
            dogMotion->isVision = true;
            direction = "left";
            dogMotion->yellowLeft = true;
            sleep(1);
            dogMotion->yellowLeft = false;
            dogMotion->isVision = false;
            cout << "Area: " << direction << endl;
        }
        if (leftCount > rightCount) // delete isTime()
        {
            dogMotion->isVision = true;
            direction = "right";
            dogMotion->yellowRight = true;
            sleep(1);
            dogMotion->yellowRight = false;
            dogMotion->isVision = false;
            cout << "Area: " << direction << endl;
        }
    }
    // else
    // {
    //     //direction = "normal";
    // }
}


void DogVision::Start() {
    int deviceNode = 0; ///< default 0 -> /dev/video0
    cv::Size frameSize(1856, 800); ///< default frame size 1856x800
    int fps = 30; ///< default camera fps: 30
    if (!cam.isOpened())   ///< get camera open state
        exit(EXIT_FAILURE);

    cam.setRawFrameSize(frameSize); ///< set camera frame size
    cam.setRawFrameRate(fps);       ///< set camera camera fps
    cam.setRectFrameSize(cv::Size(frameSize.width >> 2, frameSize.height >> 1)); ///< set camera rectify frame size
    cam.startCapture(); ///< disable image h264 encoding and share memory sharing

    usleep(500000);
    while (cam.isOpened()) {
        cv::Mat left, right, film, film_clone;
        if (!cam.getRectStereoFrame(left, right, film)) { ///< get rectify left,right frame
            usleep(1000);
            continue;
        }
        film_clone = film.clone();
        processImage(film);
        detectYellow(film_clone);
        char key = cv::waitKey(10);
        if (key == 27) // press ESC key
            break;
    }
    cam.stopCapture();
}


