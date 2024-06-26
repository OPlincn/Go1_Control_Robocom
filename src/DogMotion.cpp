#include "DogMotion.h"


DogMotion::DogMotion(uint8_t level) : safe(LeggedType::Go1), udp(level, 8090, "192.168.123.161", 8082) {
    udp.InitCmdData(cmd);
    // 在构造类的时候开始接受信号

};


void DogMotion::UDPRecv()
{
    // wait();
    udp.Recv();
}

void DogMotion::UDPSend()
{
    // wait();
    udp.Send();
}

void DogMotion::getInitIMU(int time) {
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;
    while (time--) {
        if (!isFirst && initYaw==0) {
            udp.GetRecv(state);
            initYaw = state.imu.rpy[2] * (180.0 / M_PI); // 获得初始yaw值
            if (initYaw != 0) {
                isFirst = true;
                cout << "初始Yaw值为： " << initYaw << endl;
            }
        }
        //cmd.velocity[0] = 0.1f;
//            yawCorrect(90);
        usleep(10000);
        udp.SetSend(cmd); // really necessary?
    }
}

void DogMotion::initState()
{
    cmd.mode = 0;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 0;
    cmd.speedLevel = 0;
    cmd.footRaiseHeight = 0;
    cmd.bodyHeight = 0;
    cmd.euler[0]  = 0;
    cmd.euler[1] = 0;
    cmd.euler[2] = 0;
    cmd.velocity[0] = 0.0f;
    cmd.velocity[1] = -0.0125f;//DO NOT ADJUST DEFAULT VALUE
    cmd.yawSpeed = 0.012f;
    cmd.reserve = 0;
}

void DogMotion::visionControl() {
    cmd.mode = 2;
    cmd.gaitType = 1;
    cmd.euler[0]  = 0;
    cmd.euler[1] = 0;
    cmd.euler[2] = 0;
    cmd.velocity[0] = 0.0f;
    cmd.velocity[1] = 0.0f;
    cmd.yawSpeed = 0.012f;
    // below are last competition's vision code, order to let the dog in charge of the vision
    if(isVision && midShiftRight && !yellowRight && !yellowLeft && !greenLeft && !greenRight) {
        cmd.velocity[1] = -0.08f;
        cmd.velocity[0] = 0.05f;
    }
    if(isVision && midShiftLeft && !yellowRight && !yellowLeft && !greenLeft && !greenRight) {
        cmd.velocity[1] = 0.08f;
        cmd.velocity[0] = 0.05f;
    }
    if(isVision && yellowRight && !greenLeft && !greenRight) {
        cmd.velocity[0] = 0.05f;
        cmd.yawSpeed = -0.1f;
        cmd.velocity[1] = -0.2f;
    }
    if(isVision && yellowLeft && !greenLeft && !greenRight) {
        cmd.velocity[0] = 0.05f;
        cmd.yawSpeed = 0.1f;
        cmd.velocity[1] = +0.2f;
    }
    if (isVision && greenRight) {
        cmd.velocity[1] = -0.4f;
    }
    if (isVision && greenLeft) {
        cmd.velocity[1] = 0.4f;
    }

};


void DogMotion::yawCorrect(int deflection=0) {
    udp.GetRecv(state);
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;
    int baseYaw = this->initYaw + deflection; // baseYaw 就是初始yaw + 偏移 yaw
    if (baseYaw > 180) {
        baseYaw -=360;
    } else if (baseYaw < -180) {
        baseYaw += 360;
    }
    int nowYaw = state.imu.rpy[2] * (180.0 / M_PI);
    int diff = nowYaw - baseYaw;
    if (diff > 180) {
        diff -= 360;
    } else if(diff < -180) {
        diff += 360;
    }
    if (abs(diff) > 3 && nowYaw !=0 && baseYaw != 0) {
        // yaw left + , right -
        cout << "nowYaw: " << nowYaw << "  now baseYaw: " << baseYaw << endl;

        // zheng ni, fu shun
        cout << "现在相对baseYaw的偏角为: " << diff << " 正在进行修正 ！" << endl;
        if (diff > 0) {
            cmd.yawSpeed = -0.6f;
        } else {
            cmd.yawSpeed = 0.6f;
        }
    } else cmd.yawSpeed = 0.012f;
}

void DogMotion::GoForward(int time, bool useIMU=false, int deflection=0, bool useVision=true)
{
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;

    while(time--)
    {

        if (isVision&&useVision) {
            visionControl();
            ++time;
        } else {
            printf("Go Forward %d times\n",time);
            cmd.velocity[0] = 0.5f;
            cmd.velocity[1] = -0.0125f;//DO NOT ADJUST DEFAULT VALUE
            cmd.yawSpeed = 0.012f;
            if (useIMU) yawCorrect(deflection);
        }
        udp.SetSend(cmd);
        usleep(10000);
    }
}

void DogMotion::RightTranslation(int time,bool useVision=true)
{
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;
    while(time--)
    {
        if (isVision&&useVision) {
            visionControl();
            ++time;
        } else {
            printf("Right Translation %d times\n",time);
            cmd.velocity[1] = -0.4f;
        }
        udp.SetSend(cmd);
        usleep(10000);
    }
}

void DogMotion::LeftTranslation(int time,bool useVision=true)
{
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;//if dog go foward continuously when translation, change mode to another digit

    while(time--)
    {
        if (isVision&&useVision) {
            visionControl();
            ++time;
        } else {
            printf("Left Translation %d times\n",time);
            cmd.velocity[1] = 0.4f;
        }
        udp.SetSend(cmd);
        usleep(10000);
    }
}


void DogMotion::TurnRight(int time,double v0, bool useVision=true)//time_yaw90=175 v0_yaw90=0.45 time_yaw45=110 v0_yaw45=0.4
{
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;
    while(time--)
    {
        if (isVision&&useVision) {
            visionControl();
            ++time;
        } else {
            printf("Turn Right %d times\n",time);
            cmd.velocity[0] = 1.0f*v0;
            cmd.yawSpeed = -1.15f;
        }

        udp.SetSend(cmd);
        usleep(10000);
    }
}


void DogMotion::TurnLeft(int time,double v0, bool useVision=true)//time_yaw90=175 v0_yaw90=0.45 time_yaw45=110 v0_yaw45=0.4
{
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;
    while(time--)
    {
        if (isVision&&useVision) {
            visionControl();
            ++time;
        } else {
            cout << "now yaw:" << state.imu.rpy[2] * (180.0 / 3.1415926) << endl;
            printf("Turn Left %d times\n",time);
            cmd.velocity[0] = 1.0f*v0;
            cmd.yawSpeed = 1.15f;
        }

        udp.SetSend(cmd);
        usleep(10000);
    }
}

void DogMotion::RightCircle(int time,double v0,double yaws)
{
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;
    while(time--)
    {
        if (!isVision) {
            printf("Right Circle %d times\n",time);
            cmd.velocity[0] = 1.0f*v0;
            cmd.yawSpeed = -1.0f*yaws;
        } else {
            visionControl();
            ++time;
        }

        udp.SetSend(cmd);
        usleep(10000);
    }
}

void DogMotion::LeftCircle(int time,double v0,double yaws)
{
    initState();
    cmd.mode = 2;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 1;
    while(time--)
    {
        if (!isVision) {
            printf("Left Circle %d times\n",time);
            cmd.velocity[0] = 1.0f*v0;
            cmd.yawSpeed = 1.0f*yaws;
        } else {
            visionControl();
            ++time;
        }

        udp.SetSend(cmd);
        usleep(10000);
    }
}

void DogMotion::Lean_Forward()
{
    cmd.mode=1;
    cmd.velocity[0] = 0;
    cmd.yawSpeed = 0;
    cmd.euler[1] = 0.7f;
    udp.SetSend(cmd);
    sleep(1);
    cmd.euler[0] = -0.99f;
    udp.SetSend(cmd);
    sleep(1);
    cmd.euler[1] = 0;
    cmd.euler[0] = 0;
    udp.SetSend(cmd);
    sleep(1);
}

void DogMotion::Lean_Backward()
{
    cmd.mode=1;
    cmd.velocity[0] = 0;
    cmd.yawSpeed = 0;
    cmd.euler[1] = -0.9f;
    udp.SetSend(cmd);
    sleep(1);
    cmd.euler[0] = 0.99f;
    udp.SetSend(cmd);
    sleep(1);
    cmd.euler[1] = 0;
    cmd.euler[0] = 0;
    udp.SetSend(cmd);
    sleep(1);
}

void DogMotion::Stop()
{
    cmd.mode = 0;      // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 0;
    cmd.speedLevel = 0;
    cmd.footRaiseHeight = 0;
    cmd.bodyHeight = 0;
    cmd.euler[0]  = 0;
    cmd.euler[1] = 0;
    cmd.euler[2] = 0;
    cmd.velocity[0] = 0.0f;
    cmd.velocity[1] = 0.0f;//DO NOT ADJUST DEFAULT VALUE
    cmd.yawSpeed = 0.0f;
    cmd.reserve = 0;
    printf("Stop\n");
    udp.SetSend(cmd);
    sleep(100);
}

void DogMotion::setPlanTime(int seconds) {
    this->planTimes = seconds;
}

void DogMotion::RobotControl()
{
    if(motiontime<1)
    {
        // below are the sports you want to arrange dog to do
        //from start to the end of area 1
        getInitIMU(1);
        LeftTranslation(100, false);
        GoForward(110, true, 0, false);
        TurnRight(175,0.45, false);
        GoForward(110, true, -90, true);
        LeftTranslation(60);
        motiontime+=1;

        //area 1
        LeftCircle(200,0.5,1.4);
        Lean_Forward();
        LeftCircle(325,0.5,1.5);
        RightTranslation(70);

        //from end of area 1 to start of side road
        GoForward(300);
        TurnLeft(175,0.45);
        GoForward(250);
        TurnLeft(175,0.45);
        GoForward(130);

        //side road
        TurnLeft(110,0.4);//yawleft45-in
        TurnRight(110,0.4);//yawright45-in
        GoForward(285);
        TurnRight(105,0.4);//yawright45-out
        TurnLeft(110,0.4);//yawleft45-out


        //from end of side road to start of area 2
        GoForward(145);
        TurnLeft(175,0.45);
        GoForward(350);
        LeftTranslation(35);

        //area 2
        LeftCircle(200,0.5,1.3);
        Lean_Forward();
        LeftCircle(375,0.5,1.4);

        //from end of area to start of area 3
        RightTranslation(50);
        GoForward(200);
        TurnLeft(175,0.45);
        TurnRight(175,0.45);
        GoForward(250);


        //area 3
        Lean_Backward();
        RightCircle(350,0.5,1.6);

        //from end of area 3 to start of area 4
        GoForward(230);
        TurnRight(90,0.4);
        GoForward(120);
        TurnLeft(90,0.6);
        GoForward(75);

        //area 4
        TurnLeft(175,0.45);
        GoForward(280);
        Lean_Backward();
        RightCircle(450,0.5,1.3);

        //to the end
        GoForward(310);
        TurnRight(175,0.45);
        GoForward(115);
        RightTranslation(170,false);
    }
    Stop();
}
void DogMotion::Run()
{
    // InitEnvironment();
    LoopFunc loop_control("control_loop", dt,    boost::bind(&DogMotion::RobotControl, this));
    LoopFunc loop_udpSend("udp_send",     dt, 3, boost::bind(&DogMotion::UDPSend,      this));
    LoopFunc loop_udpRecv("udp_recv",     dt, 3, boost::bind(&DogMotion::UDPRecv,      this));

    loop_udpSend.start();
    loop_udpRecv.start();
    loop_control.start();
    while(1){
        sleep(1);
    };
}
