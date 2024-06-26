#include <DogVision.h>
#include <FrontCamera.h>
#include <thread>
#define DOG_MOTION_VISION

#ifdef AGENT_TOOL
int main(int argc, char *argv[]) {
    // 主函数的代码逻辑
    int sportTime(0);
    if (argc >= 2) {
        sportTime = std::atoi(argv[1]);  // atoi用来将字符串转为数字
    }
    DogMotion myplan(HIGHLEVEL);
    myplan.setPlanTime(sportTime);
    std::thread dogGoPath(&DogMotion::Run, &myplan);
    dogGoPath.join();
    return 0;

    return 0;
}
#endif


#ifdef DOG_All_EXAMPLE
int main() {
//    DogSport sport(HIGHLEVEL);
    DogMotion myplan(HIGHLEVEL);
    FrontCamera frontCamera(&myplan);
    DogVision detector("../config/detectMid.yaml", &myplan);
    std::thread dogVisionThread(&DogVision::Start, &detector);
    std::thread dogDetectObstacle(&FrontCamera::Start, &frontCamera);
    std::thread dogGoPath(&DogMotion::Run, &myplan);
    dogVisionThread.join();
    dogGoPath.join();
    dogDetectObstacle.join();
    return 0;
}
#endif

#ifdef  DOG_MOTION_VISION
int main() {
    DogMotion myplan(HIGHLEVEL);
    DogVision detector("../config/detectMid.yaml", &myplan);
    std::thread dogVisionThread(&DogVision::Start, &detector);
    std::thread dogGoPath(&DogMotion::Run, &myplan);
    dogVisionThread.join();
    dogGoPath.join();
    return 0;
}
#endif


#ifdef  DOG_MOTION_ONLY
int main() {
    DogMotion myplan(HIGHLEVEL);
    myplan.planTimes = 5; // now RobotControl's content is GoForward, so it will go forword 5 s approximately
    myplan.Run();
    return 0;
}
#endif

#ifdef DOG_VISION_ONLY
int main() {
    DogVision detector("../config/detectMid.yaml", &myplan);
    std::thread dogVisionThread(&DogVision::Start, &detector);
    dogVisionThread.join();
    return 0;


#endif

#ifdef DOG_FRONT_ONLY
int main() {
    DogVision detector("../config/detectMid.yaml", &myplan);
    std::thread dogVisionThread(&DogVision::Start, &detector);
    dogVisionThread.join();
    return 0;


#endif

#ifdef VIDEO_TEST
int main(int argc, char** argv) {
    VideoCapture v("/Users/oplin/CLionProjects/opencv/robocom_vision/test_image/test1.mp4");
    Mat src;
    while (true) {
        if (!v.isOpened()) {
            break;
        }
        v.read(src);
        if (src.empty()) {
            break;
        }
//            camera_warrper->read_frame_rgb(src);
        DogVision detector("../config/detectMid.yaml");
        detector.createMyTrackbar();
        detector.processImage(src);
    }
    return 0;
}
#endif

