#include <stdio.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>


#include "DRFLEx.h"
using namespace DRAFramework;

#undef NDEBUG
#include <assert.h>

CDRFLEx Drfl;
bool g_bHasControlAuthority = FALSE;
bool g_TpInitailizingComplted = FALSE;
bool g_mStat = FALSE;
bool g_Stop = FALSE;
bool moving = FALSE;
string strDrl =
    "\r\n\
loop = 0\r\n\
while loop < 1003:\r\n\
 movej(posj(10,10.10,10,10.10), vel=60, acc=60)\r\n\
 movej(posj(00,00.00,00,00.00), vel=60, acc=60)\r\n\
 loop+=1\r\n";

bool bAlterFlag = FALSE;

int linux_kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;

	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );


	ch = getchar();

	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

int getch()
{
    int c;
    struct termios oldattr, newattr;

    tcgetattr(STDIN_FILENO, &oldattr);           // ���� �͹̳� ���� ����
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);         // CANONICAL�� ECHO ��
    newattr.c_cc[VMIN] = 1;                      // �ּ� �Է� ���� ���� 1�� ����
    newattr.c_cc[VTIME] = 0;                     // �ּ� �б� ��� �ð��� 0���� ����
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);  // �͹̳ο� ���� �Է�
    c = getchar();                               // Ű���� �Է� ����
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);  // ������ �������� ����
    return c;
}

void OnTpInitializingCompleted() {
  // Tp �ʱ�ȭ ���� ����� ��û.
  g_TpInitailizingComplted = TRUE;
  Drfl.ManageAccessControl(MANAGE_ACCESS_CONTROL_FORCE_REQUEST);
}

void OnHommingCompleted() {
  // 50msec �̳� �۾��� ������ ��.
  cout << "homming completed" << endl;
}

void OnProgramStopped(const PROGRAM_STOP_CAUSE) {
  assert(Drfl.PlayDrlStop(STOP_TYPE_SLOW));
  // 50msec �̳� �۾��� ������ ��.
  // assert(Drfl.SetRobotMode(ROBOT_MODE_MANUAL));
  cout << "program stopped" << endl;
}

void OnMonitoringDataCB(const LPMONITORING_DATA pData) {
  // 50msec �̳� �۾��� ������ ��.

  return;
  cout << "# monitoring 0 data " << pData->_tCtrl._tTask._fActualPos[0][0]
       << pData->_tCtrl._tTask._fActualPos[0][1]
       << pData->_tCtrl._tTask._fActualPos[0][2]
       << pData->_tCtrl._tTask._fActualPos[0][3]
       << pData->_tCtrl._tTask._fActualPos[0][4]
       << pData->_tCtrl._tTask._fActualPos[0][5] << endl;
}

void OnMonitoringDataExCB(const LPMONITORING_DATA_EX pData) {
  return;
  cout << "# monitoring 1 data " << pData->_tCtrl._tWorld._fTargetPos[0]
       << pData->_tCtrl._tWorld._fTargetPos[1]
       << pData->_tCtrl._tWorld._fTargetPos[2]
       << pData->_tCtrl._tWorld._fTargetPos[3]
       << pData->_tCtrl._tWorld._fTargetPos[4]
       << pData->_tCtrl._tWorld._fTargetPos[5] << endl;
}

void OnMonitoringCtrlIOCB(const LPMONITORING_CTRLIO pData) {
  return;
  cout << "# monitoring ctrl 0 data" << endl;
  for (int i = 0; i < 16; i++) {
    cout << (int)pData->_tInput._iActualDI[i] << endl;
  }
}

void OnMonitoringCtrlIOExCB(const LPMONITORING_CTRLIO_EX pData) {
  return;
  cout << "# monitoring ctrl 1 data" << endl;
  for (int i = 0; i < 16; i++) {
    cout << (int)pData->_tInput._iActualDI[i] << endl;
  }
  for (int i = 0; i < 16; i++) {
    cout << (int)pData->_tOutput._iTargetDO[i] << endl;
  }
}

void OnMonitoringStateCB(const ROBOT_STATE eState) {
  // 50msec �̳� �۾��� ������ ��.
  switch ((unsigned char)eState) {
#if 0  // TP �ʱ�ȭ�� ����ϴ� ���������� API ���������� ������� ����.(TP����
       // �ܵ� ����� ���, ���)
    case STATE_NOT_READY:
        if (g_bHasControlAuthority) Drfl.SetRobotControl(CONTROL_INIT_CONFIG);
        break;
    case STATE_INITIALIZING:
        // add initalizing logic
        if (g_bHasControlAuthority) Drfl.SetRobotControl(CONTROL_ENABLE_OPERATION);
        break;
#endif
    case STATE_STANDBY:
    case STATE_MOVING:
    case STATE_TEACHING:
      break;
    case STATE_SAFE_OFF:
      // cout << "STATE_SAFE_OFF1" << endl;
      if (g_bHasControlAuthority) {
        // cout << "STATE_SAFE_OFF2" << endl;
        Drfl.SetRobotControl(CONTROL_SERVO_ON);
        Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
      }
      break;
    case STATE_SAFE_STOP:
      // if (Drfl.get_robot_mode()==ROBOT_MODE_AUTONOMOUS) {
      // // Replays the program after conversion of state if the current state is an automatic mode
      // Drfl.set_safe_stop_reset_type(SAFE_STOP_RESET_TYPE_PROGRAM_RESUME);
      // Drfl.set_robot_control(CONTROL_RESET_SAFET_STOP);
      // Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
      // }
      // else {
      // // Converts to STATE_STANDBY if the current state is a manual mode
      // Drfl.set_robot_control(CONTROL_RESET_SAFET_STOP);
      // Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
      // }
      cout<<"It has been stopped"<<endl;
      if (g_bHasControlAuthority) {
        Drfl.SetSafeStopResetType(SAFE_STOP_RESET_TYPE_DEFAULT);
        Drfl.SetRobotControl(CONTROL_RESET_SAFET_STOP);
        Drfl.SetRobotControl(CONTROL_SERVO_ON);
        Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);

      // Drfl.SetRobotControl(CONTROL_SERVO_ON);
      // Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
      }
      break;
    case STATE_EMERGENCY_STOP:
      // popup
      break;
    case STATE_RECOVERY:
      // Drfl.SetRobotControl(CONTROL_RESET_RECOVERY);
      Drfl.SetRobotControl(CONTROL_SERVO_ON);
      Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
      break;
    case STATE_SAFE_STOP2:
      if (g_bHasControlAuthority){
        // Drfl.SetRobotControl(CONTROL_RECOVERY_SAFE_STOP);
        Drfl.SetRobotControl(CONTROL_SERVO_ON);
        Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
      }
      break;
    case STATE_SAFE_OFF2:
      if (g_bHasControlAuthority) {
        // Drfl.SetRobotControl(CONTROL_RECOVERY_SAFE_OFF);
        Drfl.SetRobotControl(CONTROL_SERVO_ON);
        Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
      }
      break;

    default:
      break;
  }
  return;
  cout << "current state: " << (int)eState << endl;
}

void OnMonitroingAccessControlCB(
    const MONITORING_ACCESS_CONTROL eTrasnsitControl) {
  // 50msec �̳� �۾��� ������ ��.

  switch (eTrasnsitControl) {
    case MONITORING_ACCESS_CONTROL_REQUEST:
      assert(Drfl.ManageAccessControl(MANAGE_ACCESS_CONTROL_RESPONSE_NO));
      // Drfl.ManageAccessControl(MANAGE_ACCESS_CONTROL_RESPONSE_YES);
      break;
    case MONITORING_ACCESS_CONTROL_GRANT:
      g_bHasControlAuthority = TRUE;
      // cout << "GRANT1" << endl;
      // cout << "MONITORINGCB : " << (int)Drfl.GetRobotState() << endl;
      OnMonitoringStateCB(Drfl.GetRobotState());
      // cout << "GRANT2" << endl;
      break;
    case MONITORING_ACCESS_CONTROL_DENY:
    case MONITORING_ACCESS_CONTROL_LOSS:
      g_bHasControlAuthority = FALSE;
      if (g_TpInitailizingComplted) {
        // assert(Drfl.ManageAccessControl(MANAGE_ACCESS_CONTROL_REQUEST));
        Drfl.ManageAccessControl(MANAGE_ACCESS_CONTROL_FORCE_REQUEST);
      }
      break;
    default:
      break;
  }
}

void OnLogAlarm(LPLOG_ALARM tLog) {
  g_mStat = true;
  cout << "Alarm Info: "
       << "group(" << (unsigned int)tLog->_iGroup << "), index("
       << tLog->_iIndex << "), param(" << tLog->_szParam[0] << "), param("
       << tLog->_szParam[1] << "), param(" << tLog->_szParam[2] << ")" << endl;
}

void OnTpPopup(LPMESSAGE_POPUP tPopup) {
  cout << "Popup Message: " << tPopup->_szText << endl;
  cout << "Message Level: " << tPopup->_iLevel << endl;
  cout << "Button Type: " << tPopup->_iBtnType << endl;
}

void OnTpLog(const char* strLog) { cout << "Log Message: " << strLog << endl; }

void OnTpProgress(LPMESSAGE_PROGRESS tProgress) {
  cout << "Progress cnt : " << (int)tProgress->_iTotalCount << endl;
  cout << "Current cnt : " << (int)tProgress->_iCurrentCount << endl;
}

void OnTpGetuserInput(LPMESSAGE_INPUT tInput) {
  cout << "User Input : " << tInput->_szText << endl;
  cout << "Data Type : " << (int)tInput->_iType << endl;
}

void OnRTMonitoringData(LPRT_OUTPUT_DATA_LIST tData)
{
   // static int td = 0;
   // if (td++ == 1000) {
   // 	td = 0;
   // 	printf("timestamp : %.3f\n", tData->time_stamp);
   // 	printf("joint : %f %f %f %f %f %f\n", tData->actual_joint_position[0], tData->actual_joint_position[1], tData->actual_joint_position[2], tData->actual_joint_position[3], tData->actual_joint_position[4], tData->actual_joint_position[5]);
		// printf("q = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
		// 		tData->actual_joint_position[0], tData->actual_joint_position[1], tData->actual_joint_position[2],
		// 		tData->actual_joint_position[3], tData->actual_joint_position[4], tData->actual_joint_position[5]);
		// printf("q_dot = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
		// 		tData->actual_joint_velocity[0], tData->actual_joint_velocity[1], tData->actual_joint_velocity[2],
		// 		tData->actual_joint_velocity[3], tData->actual_joint_velocity[4], tData->actual_joint_velocity[5]);
		// printf("trq_g = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
		// 		tData->gravity_torque[0], tData->gravity_torque[1], tData->gravity_torque[2],
		// 		tData->gravity_torque[3], tData->gravity_torque[4], tData->gravity_torque[5]);
   //  printf("tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
  	// 		tData->actual_joint_torque[0], tData->actual_joint_torque[1], tData->actual_joint_torque[2],
  	// 		tData->actual_joint_torque[3], tData->actual_joint_torque[4], tData->actual_joint_torque[5]);
   // cout <<"Robot Mode : " << tData->robot_mode << ", Robot State : " << tData->robot_state << ", Control Mode : " << tData->control_mode<<endl;
   //
   //
   //  printf("\n\n\n");
   // }
}


uint32_t ThreadFunc(void* arg) {
	printf("start ThreadFunc\n");

	while (true) {
		if(linux_kbhit()){
			char ch = getch();
			switch (ch) {
				case 's': {
					printf("Stop!\n");
					g_Stop = true;
					Drfl.MoveStop(STOP_TYPE_SLOW);
				} break;
				case 'p': {
					printf("Pause!\n");
					Drfl.MovePause();
				} break;
				case 'r': {
					printf("Resume!\n");
					Drfl.MoveResume();
				} break;
			}
		}

		//Sleep(100);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cout << "exit ThreadFunc" << std::endl;

	return 0;
}

void OnDisConnected() {
  // while (!Drfl.open_connection("192.168.0.114")) {
  while (!Drfl.open_connection("192.168.12.246")) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

struct PlanParam
{
	float time;

	float ps[6];
	float vs[6];
	float as[6];
	float pf[6];
	float vf[6];
	float af[6];

	float A0[6];
	float A1[6];
	float A2[6];
	float A3[6];
	float A4[6];
	float A5[6];
};

struct TraParam
{
	float time;

	float pos[6];
	float vel[6];
	float acc[6];
};

void TrajectoryPlan(PlanParam* plan)
{
    float ps[6],vs[6],as[6];
    float pf[6],vf[6],af[6];
    float tf;

	tf = plan->time;

    for(int i=0; i<6; i++)
    {
        ps[i] = plan->ps[i];
        vs[i] = plan->vs[i];
        as[i] = plan->as[i];
        pf[i] = plan->pf[i];
        vf[i] = plan->vf[i];
        af[i] = plan->af[i];
    }

    for(int i=0; i<6; i++)
    {
        plan->A0[i] = ps[i];
        plan->A1[i] = vs[i];
        plan->A2[i] = as[i]/2;
        plan->A3[i] = (20*pf[i]-20*ps[i]-(8*vf[i]+12*vs[i])*tf-(3*as[i]-af[i])*tf*tf)/(2*tf*tf*tf);
        plan->A4[i] = (30*ps[i]-30*pf[i]+(14*vf[i]+16*vs[i])*tf+(3*as[i]-2*af[i])*tf*tf)/(2*tf*tf*tf*tf);
        plan->A5[i] = (12*pf[i]-12*ps[i]-(6*vf[i]+6*vs[i])*tf-(as[i]-af[i])*tf*tf)/(2*tf*tf*tf*tf*tf);
    }
}

void TrajectoryGenerator(PlanParam *plan, TraParam *tra)
{
    double A0[6],A1[6],A2[6],A3[6],A4[6],A5[6];
	double t = tra->time;

    for(int i=0; i<6; i++)
    {
        A0[i] = plan->A0[i];
        A1[i] = plan->A1[i];
        A2[i] = plan->A2[i];
        A3[i] = plan->A3[i];
        A4[i] = plan->A4[i];
        A5[i] = plan->A5[i];
    }

    for(int i=0; i<6; i++)
    {
        tra->pos[i] = A0[i] + A1[i]*t + A2[i]*t*t + A3[i]*t*t*t + A4[i]*t*t*t*t + A5[i]*t*t*t*t*t;
        tra->vel[i] = A1[i] + 2*A2[i]*t + 3*A3[i]*t*t + 4*A4[i]*t*t*t + 5*A5[i]*t*t*t*t;
        tra->acc[i] = 2*A2[i] + 6*A3[i]*t + 12*A4[i]*t*t + 20*A5[i]*t*t*t;
    }
}

int main(int argc, char** argv) {

  // �ݹ� ���(// �ݹ� �Լ� �������� 50msec �̳� �۾��� ������ ��)
  Drfl.set_on_homming_completed(OnHommingCompleted);
  Drfl.set_on_monitoring_data(OnMonitoringDataCB);
  Drfl.set_on_monitoring_data_ex(OnMonitoringDataExCB);
  Drfl.set_on_monitoring_ctrl_io(OnMonitoringCtrlIOCB);
  Drfl.set_on_monitoring_ctrl_io_ex(OnMonitoringCtrlIOExCB);
  Drfl.set_on_monitoring_state(OnMonitoringStateCB);
  Drfl.set_on_monitoring_access_control(OnMonitroingAccessControlCB);
  Drfl.set_on_tp_initializing_completed(OnTpInitializingCompleted);
  Drfl.set_on_log_alarm(OnLogAlarm);
  Drfl.set_on_tp_popup(OnTpPopup);
  Drfl.set_on_tp_log(OnTpLog);
  Drfl.set_on_tp_progress(OnTpProgress);
  Drfl.set_on_tp_get_user_input(OnTpGetuserInput);
  Drfl.set_on_rt_monitoring_data(OnRTMonitoringData);

  Drfl.set_on_program_stopped(OnProgramStopped);
  Drfl.set_on_disconnected(OnDisConnected);
  cout<<"Setup completed"<<endl;
  // ���� ����
  // assert(Drfl.open_connection("192.168.0.114"));
  assert(Drfl.open_connection("192.168.12.246"));
  //assert(Drfl.open_connection("127.0.0.1"));  //using ros package robot simulator.
  // Drfl.manage_access_control(MANAGE_ACCESS_CONTROL_REQUEST);
  cout<<"control accessed"<<endl;
  // ���� ���� ȹ��
  SYSTEM_VERSION tSysVerion = {
      '\0',
  };
  Drfl.get_system_version(&tSysVerion);
  // ����͸� ������ ���� ����
  assert(Drfl.setup_monitoring_version(1));
  Drfl.set_robot_control(CONTROL_SERVO_ON);
  Drfl.set_digital_output(GPIO_CTRLBOX_DIGITAL_INDEX_10, TRUE);
  cout << "System version: " << tSysVerion._szController << endl;
  cout << "Library version: " << Drfl.get_library_version() << endl;

  while ((Drfl.get_robot_state() != STATE_STANDBY) || !g_bHasControlAuthority)
    // Sleep(1000);
    this_thread::sleep_for(std::chrono::milliseconds(1000));

  // ���� ��� ����

  assert(Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS));
  assert(Drfl.set_robot_system(ROBOT_SYSTEM_REAL));

  // Drfl.ConfigCreateModbus("mr1", "192.168.137.70", 552,
  // MODBUS_REGISTER_TYPE_HOLDING_REGISTER, 3, 5);

  typedef enum {
    EXAMPLE_JOG,
    EXAMPLE_HOME,
    EXAMPLE_MOVEJ_ASYNC,
    EXAMPLE_MOVEL_SYNC,
    EXAMPLE_MOVEJ_SYNC,
    EXAMPLE_DRL_PROGRAM,
    EXAMPLE_GPIO,
    EXAMPLE_MODBUS,
    EXAMPLE_LAST,
    EXAMPLE_SERVO_OFF
  } EXAMPLE;

  EXAMPLE eExample = EXAMPLE_LAST;

//TODO: figure out what theyre doing with this thread
/*  HANDLE hThread;
  DWORD dwThreadID;
  hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &dwThreadID);
  if (hThread == 0) {
    printf("Thread Error\n");
    return 0;
  }
*/

  bool bLoop = TRUE;
  while (bLoop) {
    g_mStat = false;
    g_Stop = false;
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    cout << "\nOptions\n";
    cout << "q: quit  |  0: test basic API (currently broken)  |  1: connect_rt  |  2: set_rt  \n3: start_rt  |  4: stop_rt  |  5: servoj_rt  |  6: servol_rt  \n7: speedj_rt  |  8: speedl_rt  |  9: torque_rt\n";
    cout << "input key : ";
    // char ch = _getch();
    char ch;
    cin >> ch;
    cout << ch << endl;

    switch (ch) {
      case 'q':
        bLoop = FALSE;
        break;
      case '0': {
        switch ((int)eExample) {
          case EXAMPLE_JOG:
            assert(Drfl.Jog(JOG_AXIS_JOINT_1, MOVE_REFERENCE_BASE, 0.f));
            cout << "jog stop" << endl;
            break;
          case EXAMPLE_HOME:
            assert(Drfl.Home((unsigned char)0));
            cout << "home stop" << endl;
            break;
          case EXAMPLE_MOVEJ_ASYNC:
            assert(Drfl.MoveStop(STOP_TYPE_SLOW));
            cout << "movej async stop" << endl;
            break;
          case EXAMPLE_MOVEL_SYNC:
          case EXAMPLE_MOVEJ_SYNC:
            break;
          case EXAMPLE_DRL_PROGRAM:
            assert(Drfl.PlayDrlStop(STOP_TYPE_SLOW));
            // assert(Drfl.SetRobotMode(ROBOT_MODE_MANUAL));
            // assert(Drfl.SetRobotSystem(ROBOT_SYSTEM_REAL));
            cout << "drl player stop" << endl;
            break;
          case EXAMPLE_GPIO:
            cout << "reset gpio" << endl;
            for (int i = 0; i < NUM_DIGITAL; i++) {
              assert(Drfl.SetCtrlBoxDigitalOutput((GPIO_CTRLBOX_DIGITAL_INDEX)i,
                                                  FALSE));
            }
            break;
          case EXAMPLE_MODBUS:
            cout << "reset modbus" << endl;
            assert(Drfl.SetModbusValue("mr1", 0));
            break;
          default:
            break;
        }
      } break;
      case '1':
          {
              cout << "Connecting..." << endl;
              // Drfl.connect_rt_control("192.168.0.114");
              Drfl.connect_rt_control("192.168.12.246",12347);
              //Drfl.connect_rt_control("127.0.0.1", 12345);
              //Drfl.connect_rt_control();
              cout << "Connected." << endl;
          }
          break;
      case '2':
          {
              cout << "Setting..." << endl;
              string version = "v1.0";
              float period = 0.001;
              int losscount = 4;
              Drfl.set_rt_control_output(version, period, losscount);
              cout << "Set." << endl;
          }
          break;

      case '3':
          {
            cout << "Starting..." << endl;
        	  Drfl.start_rt_control();
            cout << "Started." << endl;
          }
          break;

      case '4':
		  {
        cout << "Stopping..." << endl;
			  Drfl.stop_rt_control();
        cout << "Stopped." << endl;
		  }
      break;

      case '5':
      {
        cout << "Servoj_rt preparing" << endl;
        // float vel[6] = {10, 10, 10, 10, 10, 10};
        // float acc[6] = {100, 100, 100, 100, 100, 100};
        float vel[6] = {500, 500, 500, 500, 500, 500};
        float acc[6] = {500, 500, 500, 500, 500, 500};

        Drfl.set_velj_rt(vel);
        Drfl.set_accj_rt(acc);
        Drfl.set_velx_rt(100, 10);
        Drfl.set_accx_rt(200, 20);

        const float st=0.001; // sampling time
        const float ratio=1;

        const float None=-10000;
        float count=0;
        static float time=0;

        float home[6] = {0, 0, 0, 0, 0, 0};
        Drfl.movej(home, 60, 30);

        Drfl.set_safety_mode(SAFETY_MODE_MANUAL, SAFETY_MODE_EVENT_MOVE);
        TraParam tra;

        // Plan1
        PlanParam plan1;
        plan1.time=5;
        plan1.ps[0]=0; plan1.ps[1]=0; plan1.ps[2]=0; plan1.ps[3]=0; plan1.ps[4]=0; plan1.ps[5]=0;
        plan1.pf[0]=0; plan1.pf[1]=0; plan1.pf[2]=0; plan1.pf[3]=30; plan1.pf[4]=30; plan1.pf[5]=30;
        plan1.vs[0]=0; plan1.vs[1]=0; plan1.vs[2]=0; plan1.vs[3]=0; plan1.vs[4]=0; plan1.vs[5]=0;
        plan1.vf[0]=0; plan1.vf[1]=0; plan1.vf[2]=0; plan1.vf[3]=0; plan1.vf[4]=0; plan1.vf[5]=0;
        plan1.as[0]=0; plan1.as[1]=0; plan1.as[2]=0; plan1.as[3]=0; plan1.as[4]=0; plan1.as[5]=0;
        plan1.af[0]=0; plan1.af[1]=0; plan1.af[2]=0; plan1.af[3]=0; plan1.af[4]=0; plan1.af[5]=0;
        TrajectoryPlan(&plan1);
        cout << "servoj_rt executing" << endl;
        while(1)
        {
          time=(++count)*st;
          tra.time=time;

          TrajectoryGenerator(&plan1,&tra);

          for(int i=0; i<6; i++)
          {
            //tra.vel[i]=None;
            //tra.acc[i]=None;
          }

          if(time > plan1.time)
          {
            time=0;
            tra.pos[0]=plan1.pf[0]; tra.pos[1]=plan1.pf[1]; tra.pos[2]=plan1.pf[2]; tra.pos[3]=plan1.pf[3]; tra.pos[4]=plan1.pf[4]; tra.pos[5]=plan1.pf[5];
            //tra.pos[0]=10; tra.pos[1]=10; tra.pos[2]=10; tra.pos[3]=10; tra.pos[4]=10; tra.pos[5]=10;
            for(int i=0; i<6; i++)
            {
              tra.vel[i]=0.0;
              tra.acc[i]=0.0;
            }
            Drfl.servoj_rt(tra.pos, tra.vel, tra.acc, st*ratio);
            std::this_thread::sleep_for(std::chrono::microseconds(1000)); //not optimal but a good start at least.
            break;
          }
          cout<<"time: "<<time<<endl;
          printf("pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.pos[0],tra.pos[1],tra.pos[2],tra.pos[3],tra.pos[4],tra.pos[5]);
          printf("vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.vel[0],tra.vel[1],tra.vel[2],tra.vel[3],tra.vel[4],tra.vel[5]);
          printf("acc: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.acc[0],tra.acc[1],tra.acc[2],tra.acc[3],tra.acc[4],tra.acc[5]);
          Drfl.servoj(tra.pos, tra.vel, tra.acc, st*ratio);

          //rt_task_wait_period(NULL);
          std::this_thread::sleep_for(std::chrono::microseconds(1000)); //not optimal but a good start at least.
        }
        cout << "Servoj_rt complete" << endl;
      }
      break;

      case '6':
      {
			float vel[6] = {10, 10, 10, 10, 10, 10};
			float acc[6] = {100, 100, 100, 100, 100, 100};
          	Drfl.set_velj_rt(vel);
          	Drfl.set_accj_rt(acc);
			Drfl.set_velx_rt(100, 10);
			Drfl.set_accx_rt(200, 20);

			const float st=0.001; // sampling time
			const float ratio=1;

			const float None=-10000;
			float count=0;
			static float time=0;

			float home[6] = {0, 0, 90, 0, 90, 0};
			Drfl.movej(home, 60, 30, 2);
			Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE);

			TraParam tra;

			// Plan1
			PlanParam plan1;
			plan1.time=5;
			plan1.ps[0]=368.0; plan1.ps[1]=34.5; plan1.ps[2]=442.5; plan1.ps[3]=180; plan1.ps[4]=180; plan1.ps[5]=180;
			plan1.pf[0]=450.0; plan1.pf[1]=34.5; plan1.pf[2]=442.5; plan1.pf[3]=180; plan1.pf[4]=180; plan1.pf[5]=230;
			plan1.vs[0]=0; plan1.vs[1]=0; plan1.vs[2]=0; plan1.vs[3]=0; plan1.vs[4]=0; plan1.vs[5]=0;
			plan1.vf[0]=0; plan1.vf[1]=0; plan1.vf[2]=0; plan1.vf[3]=0; plan1.vf[4]=0; plan1.vf[5]=0;
			plan1.as[0]=0; plan1.as[1]=0; plan1.as[2]=0; plan1.as[3]=0; plan1.as[4]=0; plan1.as[5]=0;
			plan1.af[0]=0; plan1.af[1]=0; plan1.af[2]=0; plan1.af[3]=0; plan1.af[4]=0; plan1.af[5]=0;
			TrajectoryPlan(&plan1);

			while(1)
			{
				time=(++count)*st;
				tra.time=time;

				TrajectoryGenerator(&plan1,&tra);

				for(int i=0; i<6; i++)
				{
					tra.vel[i]=None;
					tra.acc[i]=None;
				}

				if(time > plan1.time)
				{
					time=0;
					tra.pos[0]=450.0; tra.pos[1]=34.5; tra.pos[2]=442.5; tra.pos[3]=180; tra.pos[4]=180; tra.pos[5]=230;
					for(int i=0; i<6; i++)
					{
						tra.vel[i]=0.0;
						tra.acc[i]=0.0;
					}
				}

				Drfl.servol_rt(tra.pos, tra.vel, tra.acc, st*ratio);

				//rt_task_wait_period(NULL);
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
			}
      } break;
      case '7': // speedj
		  {
        cout << "speedj_rt preparing" << endl;
				float vel_limit[6] = {500, 500, 500, 500, 500, 500};
				float acc_limit[6] = {500, 500, 500, 500, 500, 500};
				float vel[6] = {30, 0, 0, 0, 0, 0};
				float acc[6] = {100, 100, 100, 100, 100, 100};
				float vel0[6] = {0, 0, 0, 0, 0, 0};
				float acc0[6] = {0, 0, 0, 0, 0, 0};
				int res;
				Drfl.set_velj_rt(vel_limit);
				Drfl.set_accj_rt(acc_limit);
        Drfl.set_velx_rt(1000,1000);
        Drfl.set_accx_rt(1000,1000);

				float home[6] = {0, 0, 0, 0, 0, 0};
        float q[NUMBER_OF_JOINT] = {0.0, };
        float q_dot[NUMBER_OF_JOINT] = {0.0, };

				Drfl.movej(home, 30, 30);
				Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE); //if this is removed the robot stops frequently

				const float st=0.001; // sampling time
				const float None=-10000;
				float count=0;
        int task_motion=0;
        int flag_trj = 0;
				static float time=0;

				TraParam tra;

				// Plan1
				PlanParam plan1, plan2;

				cout << "speedj_rt executing" << endl;
				while(1)
				{
					time=(++count)*st;
					tra.time=time;

          memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
          memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
          // memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);
          // if
          //   Drfl.release_protective_stop(RELEASE_MODE_RELEASE);

          if (task_motion == 0) {
            if (flag_trj==0){
              plan1.time=5;
              plan1.ps[0]=q[0]; plan1.ps[1]=q[1]; plan1.ps[2]=q[2]; plan1.ps[3]=q[3]; plan1.ps[4]=q[4]; plan1.ps[5]=q[5];
      				plan1.pf[0]=0; plan1.pf[1]=0; plan1.pf[2]=30; plan1.pf[3]=30; plan1.pf[4]=30; plan1.pf[5]=30;
      				plan1.vs[0]=0; plan1.vs[1]=0; plan1.vs[2]=0; plan1.vs[3]=0; plan1.vs[4]=0; plan1.vs[5]=0;
      				plan1.vf[0]=0; plan1.vf[1]=0; plan1.vf[2]=0; plan1.vf[3]=0; plan1.vf[4]=0; plan1.vf[5]=0;
      				plan1.as[0]=0; plan1.as[1]=0; plan1.as[2]=0; plan1.as[3]=0; plan1.as[4]=0; plan1.as[5]=0;
      				plan1.af[0]=0; plan1.af[1]=0; plan1.af[2]=0; plan1.af[3]=0; plan1.af[4]=0; plan1.af[5]=0;
      				TrajectoryPlan(&plan1);
              flag_trj = 1;
            }
            else if (flag_trj==1){
              TrajectoryGenerator(&plan1,&tra);
            }

         		/*
       			for(int i=0; i<6; i++)
				  {
					  tra.acc[i]=None;  //flag for robot to calculate
				  }
				*/

    			res=Drfl.speedj_rt(tra.vel, tra.acc, st); //if time is zero we get similar errors
  				if(time > plan1.time)
  				{
  					count=0;
            task_motion++;
            flag_trj = 0;
  					// break;
  				}
        }
        else if (task_motion == 1) {
          if (flag_trj==0){
            plan2.time=5;
            plan2.ps[0]=q[0]; plan2.ps[1]=q[1]; plan2.ps[2]=q[2]; plan2.ps[3]=q[3]; plan2.ps[4]=q[4]; plan2.ps[5]=q[5];
            plan2.pf[0]=0; plan2.pf[1]=0; plan2.pf[2]=-30; plan2.pf[3]=-30; plan2.pf[4]=-30; plan2.pf[5]=-30;
            plan2.vs[0]=0; plan2.vs[1]=0; plan2.vs[2]=0; plan2.vs[3]=0; plan2.vs[4]=0; plan2.vs[5]=0;
            plan2.vf[0]=0; plan2.vf[1]=0; plan2.vf[2]=0; plan2.vf[3]=0; plan2.vf[4]=0; plan2.vf[5]=0;
            plan2.as[0]=0; plan2.as[1]=0; plan2.as[2]=0; plan2.as[3]=0; plan2.as[4]=0; plan2.as[5]=0;
            plan2.af[0]=0; plan2.af[1]=0; plan2.af[2]=0; plan2.af[3]=0; plan2.af[4]=0; plan2.af[5]=0;
            TrajectoryPlan(&plan2);
            flag_trj = 1;
          }
          else if (flag_trj==1){
            TrajectoryGenerator(&plan2,&tra);
          }

              res=Drfl.speedj_rt(tra.vel, tra.acc, st); //if time is zero we get similar errors
            // printf("res : %d\n",res);
        if(time > plan2.time)
        {
          count=0;
          task_motion=0;
          flag_trj = 0;
          // break;
        }
      }
      static int td = 0;
      if (td++ == 499) {
        td = 0;
        // printf("timestamp : %.3f\n", Drfl.read_data_rt()->time_stamp);
        printf("time : %.3f\n",time);
        printf("joint : %f %f %f %f %f %f\n", Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2], Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
       printf("q = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
          Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2],
          Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
       printf("q_dot = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
          Drfl.read_data_rt()->actual_joint_velocity[0], Drfl.read_data_rt()->actual_joint_velocity[1], Drfl.read_data_rt()->actual_joint_velocity[2],
          Drfl.read_data_rt()->actual_joint_velocity[3], Drfl.read_data_rt()->actual_joint_velocity[4], Drfl.read_data_rt()->actual_joint_velocity[5]);
       printf("trq_g = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
          Drfl.read_data_rt()->gravity_torque[0], Drfl.read_data_rt()->gravity_torque[1], Drfl.read_data_rt()->gravity_torque[2],
          Drfl.read_data_rt()->gravity_torque[3], Drfl.read_data_rt()->gravity_torque[4], Drfl.read_data_rt()->gravity_torque[5]);
       printf("act_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
          Drfl.read_data_rt()->actual_joint_torque[0], Drfl.read_data_rt()->actual_joint_torque[1], Drfl.read_data_rt()->actual_joint_torque[2],
          Drfl.read_data_rt()->actual_joint_torque[3], Drfl.read_data_rt()->actual_joint_torque[4], Drfl.read_data_rt()->actual_joint_torque[5]);
       printf("ext_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
             Drfl.read_data_rt()->external_joint_torque[0], Drfl.read_data_rt()->external_joint_torque[1], Drfl.read_data_rt()->external_joint_torque[2],
             Drfl.read_data_rt()->external_joint_torque[3], Drfl.read_data_rt()->external_joint_torque[4], Drfl.read_data_rt()->external_joint_torque[5]);
      cout <<"Robot Mode : " << Drfl.get_robot_mode() << ", Robot State : " << Drfl.get_robot_state() << ", Control Mode : " << Drfl.read_data_rt()->control_mode<<endl;


       printf("\n\n\n");
      }
        			  // printf("time %lf\n",time);
					//rt_task_wait_period(NULL);
        			  std::this_thread::sleep_for(std::chrono::microseconds(1000*int(1000*st)));
				}
				cout << "speedj_rt complete" << endl;
			}
		  break;
      case '8': // speedl
		  {
				float vel_limit[6] = {100, 100, 100, 100, 100, 100};
				float acc_limit[6] = {100, 100, 100, 100, 100, 100};
				float vel[6] = {50, 50, 50, 10, 10, 10};
				float acc[6] = {100, 100, 100, 100, 100, 100};
				float vel0[6] = {0, 0, 0, 0, 0, 0};
				float acc0[6] = {0, 0, 0, 0, 0, 0};

				Drfl.set_velj_rt(vel_limit);
				Drfl.set_accj_rt(acc_limit);
				Drfl.set_velx_rt(100, 10);
				Drfl.set_accx_rt(200, 20);

				const float st=0.001; // sampling time

				const float None=-10000;
				float count=0;
				static float time=0;

				float home[6] = {0, 0, 90, 0, 90, 0};
				Drfl.movej(home, 60, 30);
				Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE);

				TraParam tra;

				// Plan1
				PlanParam plan1;
				plan1.time=5;
				plan1.ps[0]=368.0; plan1.ps[1]=34.5; plan1.ps[2]=442.5; plan1.ps[3]=180; plan1.ps[4]=180; plan1.ps[5]=180;
				plan1.pf[0]=468.0; plan1.pf[1]=34.5; plan1.pf[2]=442.5; plan1.pf[3]=180; plan1.pf[4]=180; plan1.pf[5]=230;
				plan1.vs[0]=0; plan1.vs[1]=0; plan1.vs[2]=0; plan1.vs[3]=0; plan1.vs[4]=0; plan1.vs[5]=0;
				plan1.vf[0]=0; plan1.vf[1]=0; plan1.vf[2]=0; plan1.vf[3]=0; plan1.vf[4]=0; plan1.vf[5]=0;
				plan1.as[0]=0; plan1.as[1]=0; plan1.as[2]=0; plan1.as[3]=0; plan1.as[4]=0; plan1.as[5]=0;
				plan1.af[0]=0; plan1.af[1]=0; plan1.af[2]=0; plan1.af[3]=0; plan1.af[4]=0; plan1.af[5]=0;
				TrajectoryPlan(&plan1);

				while(1)
				{
					//Drfl.read_data_rt();
					time=(++count)*st;
					tra.time=time;

					TrajectoryGenerator(&plan1,&tra);

					Drfl.speedl_rt(tra.vel, tra.acc, st);

					if(time > plan1.time)
					{
						time=0;
						break;
					}

					//rt_task_wait_period(NULL);
          std::this_thread::sleep_for(std::chrono::microseconds(1000));
				}
		  }
		  break;
      case '9': // torque
		  {
        static float time = 0.0;
        float deg2rad = 3.141592653589/180;
        int count = 0;
        float q[NUMBER_OF_JOINT] = {0.0, };
        float q_dot[NUMBER_OF_JOINT] = {0.0, };
        float q_d[NUMBER_OF_JOINT] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        float q_dot_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_g[NUMBER_OF_JOINT] = {0.0, };
        float trq_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_d_tmp[NUMBER_OF_JOINT] = {0.0, };
        float kp[NUMBER_OF_JOINT] = {10.0, 10.0, 10.0, 10.0, 10.0, 10.0}; // have to tune
        float kd[NUMBER_OF_JOINT] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5}; // have to tune

        float err_q[NUMBER_OF_JOINT] = {0.0,};
        float err_q_dot[NUMBER_OF_JOINT] = {0.0,};
				TraParam tra;
        int flag_trj = 0;
        int task_motion = 0;


        float vel_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};
				float acc_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};

        Drfl.set_velj_rt(vel_limit);
				Drfl.set_accj_rt(acc_limit);
        Drfl.set_velx_rt(1000,1000);
        Drfl.set_accx_rt(1000,1000);

        float home[6] = {0, 0, 0, 0, 0, 0};
        Drfl.movej(home, 60, 30);
				Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE);

				const float st=0.001; // sampling time

        memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);

				PlanParam plan1, plan2;


				while (1)
				{
					time=(++count)*st;
          tra.time=time;

					memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);

          if (task_motion == 0) {
            if (flag_trj==0){
              plan1.time=2.5;
              plan1.ps[0]=q[0]; plan1.ps[1]=q[1]; plan1.ps[2]=q[2]; plan1.ps[3]=q[3]; plan1.ps[4]=q[4]; plan1.ps[5]=q[5];
              plan1.pf[0]=30; plan1.pf[1]=30; plan1.pf[2]=30; plan1.pf[3]=30; plan1.pf[4]=30; plan1.pf[5]=30;
              plan1.vs[0]=0; plan1.vs[1]=0; plan1.vs[2]=0; plan1.vs[3]=0; plan1.vs[4]=0; plan1.vs[5]=0;
              plan1.vf[0]=0; plan1.vf[1]=0; plan1.vf[2]=0; plan1.vf[3]=0; plan1.vf[4]=0; plan1.vf[5]=0;
              plan1.as[0]=0; plan1.as[1]=0; plan1.as[2]=0; plan1.as[3]=0; plan1.as[4]=0; plan1.as[5]=0;
              plan1.af[0]=0; plan1.af[1]=0; plan1.af[2]=0; plan1.af[3]=0; plan1.af[4]=0; plan1.af[5]=0;
              TrajectoryPlan(&plan1);
              flag_trj = 1;
            }
            else if (flag_trj==1){
              TrajectoryGenerator(&plan1,&tra);
            }
          if(time > plan1.time)
          {
            count=0;
            task_motion++;
            flag_trj = 0;
            // break;
          }
        }
        else if (task_motion == 1) {
          if (flag_trj==0){
            plan2.time=2.5;
            plan2.ps[0]=q[0]; plan2.ps[1]=q[1]; plan2.ps[2]=q[2]; plan2.ps[3]=q[3]; plan2.ps[4]=q[4]; plan2.ps[5]=q[5];
            plan2.pf[0]=-30; plan2.pf[1]=-30; plan2.pf[2]=-30; plan2.pf[3]=-30; plan2.pf[4]=-30; plan2.pf[5]=-30;
            plan2.vs[0]=0; plan2.vs[1]=0; plan2.vs[2]=0; plan2.vs[3]=0; plan2.vs[4]=0; plan2.vs[5]=0;
            plan2.vf[0]=0; plan2.vf[1]=0; plan2.vf[2]=0; plan2.vf[3]=0; plan2.vf[4]=0; plan2.vf[5]=0;
            plan2.as[0]=0; plan2.as[1]=0; plan2.as[2]=0; plan2.as[3]=0; plan2.as[4]=0; plan2.as[5]=0;
            plan2.af[0]=0; plan2.af[1]=0; plan2.af[2]=0; plan2.af[3]=0; plan2.af[4]=0; plan2.af[5]=0;
            TrajectoryPlan(&plan2);
            flag_trj = 1;
          }
          else if (flag_trj==1){
            TrajectoryGenerator(&plan2,&tra);
          }

        if(time > plan2.time)
        {
          count=0;
          task_motion=0;
          flag_trj = 0;
          // break;
        }
      }

      for (int i=0; i<6; i++) {
          //trq_d[i] = (float)trq_g[i]+kp[i]*((float)tra.pos[i]-q[i])+kd[i]*((float)tra.vel[i]-q_dot[i]);

          err_q[i] = tra.pos[i]-q[i];
          err_q_dot[i] = tra.vel[i]-q_dot[i];
          trq_d_tmp[i] = trq_g[i]+kp[i]*err_q[i]+kd[i]*err_q_dot[i];
          // trq_d_tmp[i] = trq_g[i];
          // trq_d[i]=trq_g[i];
          // trq_d[i] = trq_d_tmp[i];
          switch (i) {
            case 0:
            {
              if (abs(trq_d_tmp[i]) < 244)
                trq_d[i] = trq_d_tmp[i];
              else
                trq_d[i] = 244;
            }
            break;
            case 1:
            {
              if (abs(trq_d_tmp[i]) < 244)
                trq_d[i] = trq_d_tmp[i];
              else
                trq_d[i] = 244;
            }
            break;
            case 2:
            {
              if (abs(trq_d_tmp[i]) < 143.5)
                trq_d[i] = trq_d_tmp[i];
              else
                trq_d[i] = 143.5;
            }
            break;
            default:
            {
              if (abs(trq_d_tmp[i]) < 74.5)
                trq_d[i] = trq_d_tmp[i];
              else
                trq_d[i] = 74.5;
            }
            break;
          }
      }
      Drfl.torque_rt(trq_d, st);

          // cout<<"time: "<<time<<endl;
          // printf("cur_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q[0],q[1],q[2],q[3],q[4],q[5]);
          // printf("cur_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q_dot[0],q_dot[1],q_dot[2],q_dot[3],q_dot[4],q_dot[5]);
          // printf("tar_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.pos[0],tra.pos[1],tra.pos[2],tra.pos[3],tra.pos[4],tra.pos[5]);
          // printf("tar_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.vel[0],tra.vel[1],tra.vel[2],tra.vel[3],tra.vel[4],tra.vel[5]);
          // printf("tar_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_d_tmp[0],trq_d_tmp[1],trq_d_tmp[2],trq_d_tmp[3],trq_d_tmp[4],trq_d_tmp[5]);
          // printf("grav_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_g[0],trq_g[1],trq_g[2],trq_g[3],trq_g[4],trq_g[5]);
          // printf("\n\n");

					//rt_task_wait_period(NULL);
          std::this_thread::sleep_for(std::chrono::microseconds(1000*int(1000*st)));
				}
		  }
		  break;
      case 't': // torque
		  {
        static float time = 0.0;
        const float st=0.001; // sampling time
        float deg2rad = 3.141592653589/180;
        int count = 0;
        float q[NUMBER_OF_JOINT] = {0.0, };
        float q_dot[NUMBER_OF_JOINT] = {0.0, };
        float q_d[NUMBER_OF_JOINT] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        float q_dot_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_g[NUMBER_OF_JOINT] = {0.0, };
        float trq_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_d_tmp[NUMBER_OF_JOINT] = {0.0, };
        float kp[NUMBER_OF_JOINT] = {10.0, 10.0, 10.0, 5.0, 5.0, 5.0}; // have to tune
        float kd[NUMBER_OF_JOINT] = {0.5, 0.5, 0.5, 0.25, 0.25, 0.25}; // have to tune

        float tor_ext_tmp[NUMBER_OF_JOINT] = {0.0, };
        float buf_tor_ext[NUMBER_OF_JOINT] = {0.0, };
        float tor_ext[NUMBER_OF_JOINT] = {0.0, };
        float frq_cut_off = 19; //100Hz
        float alpha = (frq_cut_off*st)/(1+frq_cut_off*st);

        float err_q[NUMBER_OF_JOINT] = {0.0,};
        float err_q_dot[NUMBER_OF_JOINT] = {0.0,};
				TraParam tra;
        int flag_trj = 0;
        int task_motion = 0;


        float vel_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};
				float acc_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};

        Drfl.set_velj_rt(vel_limit);
				Drfl.set_accj_rt(acc_limit);
        Drfl.set_velx_rt(1000,1000);
        Drfl.set_accx_rt(1000,1000);

        float home[6] = {0, 0, 0, 0, 0, 0};
        Drfl.movej(home, 60, 30);
        Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
				Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE);
        Drfl.change_collision_sensitivity(0);


        memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(buf_tor_ext, Drfl.read_data_rt()->external_joint_torque, sizeof(float)*NUMBER_OF_JOINT);

				PlanParam plan1, plan2;


				while (1)
				{
					time=(++count)*st;
          tra.time=time;
          if (Drfl.get_robot_state()==STATE_SAFE_STOP||Drfl.get_robot_state()==STATE_SAFE_OFF)
          {
            Drfl.set_robot_mode(ROBOT_MODE_AUTONOMOUS);
    				Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE);
            Drfl.change_collision_sensitivity(0);
          }
					memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);
          memcpy(tor_ext_tmp, Drfl.read_data_rt()->external_joint_torque, sizeof(float)*NUMBER_OF_JOINT);


          for (int i=0; i<6; i++) {
              //trq_d[i] = (float)trq_g[i]+kp[i]*((float)tra.pos[i]-q[i])+kd[i]*((float)tra.vel[i]-q_dot[i]);

              err_q[i] = q_d[i]-q[i];
              err_q_dot[i] = q_dot_d[i]-q_dot[i];
              tor_ext[i] = alpha*tor_ext_tmp[i] + (1-alpha)*buf_tor_ext[i];
              trq_d_tmp[i] = trq_g[i]+kp[i]*err_q[i]+kd[i]*err_q_dot[i]+2.5*tor_ext[i];
              // trq_d_tmp[i] = trq_g[i];
              // trq_d[i]=trq_g[i];

              // Saturation
              if (abs(trq_d_tmp[i]) < 85)
                trq_d[i] = trq_d_tmp[i];
              else
                trq_d[i] = 85;

          }



          Drfl.torque_rt(trq_d, st);






          // cout<<"time: "<<time<<endl;
          // printf("cur_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q[0],q[1],q[2],q[3],q[4],q[5]);
          // printf("cur_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q_dot[0],q_dot[1],q_dot[2],q_dot[3],q_dot[4],q_dot[5]);
          // printf("tar_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.pos[0],tra.pos[1],tra.pos[2],tra.pos[3],tra.pos[4],tra.pos[5]);
          // printf("tar_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.vel[0],tra.vel[1],tra.vel[2],tra.vel[3],tra.vel[4],tra.vel[5]);
          // printf("tar_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_d_tmp[0],trq_d_tmp[1],trq_d_tmp[2],trq_d_tmp[3],trq_d_tmp[4],trq_d_tmp[5]);
          // printf("grav_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_g[0],trq_g[1],trq_g[2],trq_g[3],trq_g[4],trq_g[5]);
          // printf("\n\n");

          static int td = 0;
          if (td++ == 1) {
          	td = 0;
          	// printf("timestamp : %.3f\n", Drfl.read_data_rt()->time_stamp);
            printf("time : %.3f\n",time);
          	printf("joint : %f %f %f %f %f %f\n", Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2], Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
           printf("q = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2],
           		Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
           printf("q_dot = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_velocity[0], Drfl.read_data_rt()->actual_joint_velocity[1], Drfl.read_data_rt()->actual_joint_velocity[2],
           		Drfl.read_data_rt()->actual_joint_velocity[3], Drfl.read_data_rt()->actual_joint_velocity[4], Drfl.read_data_rt()->actual_joint_velocity[5]);
           printf("trq_g = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->gravity_torque[0], Drfl.read_data_rt()->gravity_torque[1], Drfl.read_data_rt()->gravity_torque[2],
           		Drfl.read_data_rt()->gravity_torque[3], Drfl.read_data_rt()->gravity_torque[4], Drfl.read_data_rt()->gravity_torque[5]);
           printf("act_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_torque[0], Drfl.read_data_rt()->actual_joint_torque[1], Drfl.read_data_rt()->actual_joint_torque[2],
           		Drfl.read_data_rt()->actual_joint_torque[3], Drfl.read_data_rt()->actual_joint_torque[4], Drfl.read_data_rt()->actual_joint_torque[5]);
           printf("ext_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
                 Drfl.read_data_rt()->external_joint_torque[0], Drfl.read_data_rt()->external_joint_torque[1], Drfl.read_data_rt()->external_joint_torque[2],
                 Drfl.read_data_rt()->external_joint_torque[3], Drfl.read_data_rt()->external_joint_torque[4], Drfl.read_data_rt()->external_joint_torque[5]);
          cout <<"Robot Mode : " << Drfl.get_robot_mode() << ", Robot State : " << Drfl.get_robot_state() << ", Control Mode : " << Drfl.get_control_mode()<<endl;


           printf("\n\n\n");
          }
					//rt_task_wait_period(NULL);
          std::this_thread::sleep_for(std::chrono::microseconds(1000*int(1000*st)));
				}
		  }
		  break;
      case 'g': // torque
		  {
        static float time = 0.0;
				const float st=0.001; // sampling time

        float deg2rad = 3.141592653589/180;
        int count = 0;
        float q[NUMBER_OF_JOINT] = {0.0, };
        float q_dot[NUMBER_OF_JOINT] = {0.0, };
        float q_d[NUMBER_OF_JOINT] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        float q_dot_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_g[NUMBER_OF_JOINT] = {0.0, };
        float trq_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_d_tmp[NUMBER_OF_JOINT] = {0.0, };
        float kp[NUMBER_OF_JOINT] = {1.0, 1.0, 1.0, 0.5, 0.5, 0.5}; // have to tune
        float kd[NUMBER_OF_JOINT] = {0.05, 0.05, 0.05, 0.025, 0.025, 0.025}; // have to tune

        float tor_ext_tmp[NUMBER_OF_JOINT] = {0.0, };
        float buf_tor_ext[NUMBER_OF_JOINT] = {0.0, };
        float tor_ext[NUMBER_OF_JOINT] = {0.0, };
        float frq_cut_off = 20; //100Hz
        float alpha = (frq_cut_off*st)/(1+frq_cut_off*st);


        float err_q[NUMBER_OF_JOINT] = {0.0,};
        float err_q_dot[NUMBER_OF_JOINT] = {0.0,};
				TraParam tra;
        int flag_trj = 0;
        int task_motion = 0;


        float vel_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};
				float acc_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};

        Drfl.set_velj_rt(vel_limit);
				Drfl.set_accj_rt(acc_limit);
        Drfl.set_velx_rt(1000,1000);
        Drfl.set_accx_rt(1000,1000);

				Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE);


        memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(buf_tor_ext, Drfl.read_data_rt()->external_joint_torque, sizeof(float)*NUMBER_OF_JOINT);

				PlanParam plan1, plan2;



				while (1)
				{
					time=(++count)*st;
          tra.time=time;

					memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);
          memcpy(tor_ext_tmp, Drfl.read_data_rt()->external_joint_torque, sizeof(float)*NUMBER_OF_JOINT);


          for (int i=0; i<6; i++) {
              //trq_d[i] = (float)trq_g[i]+kp[i]*((float)tra.pos[i]-q[i])+kd[i]*((float)tra.vel[i]-q_dot[i]);

              err_q[i] = q_d[i]-q[i];
              err_q_dot[i] = q_dot_d[i]-q_dot[i];
              tor_ext[i] = alpha*tor_ext_tmp[i] + (1-alpha)*buf_tor_ext[i];
              trq_d_tmp[i] = trq_g[i]+2.5*tor_ext[i];
              // trq_d_tmp[i] = trq_g[i];
              // trq_d_tmp[i] = trq_g[i];
              // trq_d[i]=trq_g[i];

              // trq_d[i] = trq_d_tmp[i];

              // Saturation
              switch (i) {
                case 0:
                {
                  if (abs(trq_d_tmp[i]) < 244)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 244;
                }
                break;
                case 1:
                {
                  if (abs(trq_d_tmp[i]) < 244)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 244;
                }
                break;
                case 2:
                {
                  if (abs(trq_d_tmp[i]) < 143.5)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 143.5;
                }
                break;
                default:
                {
                  if (abs(trq_d_tmp[i]) < 74.5)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 74.5;
                }
                break;
              }


          }
          Drfl.torque_rt(trq_d, st);

          memcpy(buf_tor_ext, tor_ext, sizeof(float)*NUMBER_OF_JOINT);




          // cout<<"time: "<<time<<endl;
          // printf("cur_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q[0],q[1],q[2],q[3],q[4],q[5]);
          // printf("cur_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q_dot[0],q_dot[1],q_dot[2],q_dot[3],q_dot[4],q_dot[5]);
          // printf("tar_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.pos[0],tra.pos[1],tra.pos[2],tra.pos[3],tra.pos[4],tra.pos[5]);
          // printf("tar_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.vel[0],tra.vel[1],tra.vel[2],tra.vel[3],tra.vel[4],tra.vel[5]);
          // printf("tar_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_d_tmp[0],trq_d_tmp[1],trq_d_tmp[2],trq_d_tmp[3],trq_d_tmp[4],trq_d_tmp[5]);
          // printf("grav_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_g[0],trq_g[1],trq_g[2],trq_g[3],trq_g[4],trq_g[5]);
          // printf("\n\n");

          static int td = 0;
          if (td++ == 499) {
          	td = 0;
          	// printf("timestamp : %.3f\n", Drfl.read_data_rt()->time_stamp);
            printf("time : %.3f\n",time);
          	printf("joint : %f %f %f %f %f %f\n", Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2], Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
           printf("q = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2],
           		Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
           printf("q_dot = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_velocity[0], Drfl.read_data_rt()->actual_joint_velocity[1], Drfl.read_data_rt()->actual_joint_velocity[2],
           		Drfl.read_data_rt()->actual_joint_velocity[3], Drfl.read_data_rt()->actual_joint_velocity[4], Drfl.read_data_rt()->actual_joint_velocity[5]);
           printf("trq_g = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->gravity_torque[0], Drfl.read_data_rt()->gravity_torque[1], Drfl.read_data_rt()->gravity_torque[2],
           		Drfl.read_data_rt()->gravity_torque[3], Drfl.read_data_rt()->gravity_torque[4], Drfl.read_data_rt()->gravity_torque[5]);
           printf("act_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_torque[0], Drfl.read_data_rt()->actual_joint_torque[1], Drfl.read_data_rt()->actual_joint_torque[2],
           		Drfl.read_data_rt()->actual_joint_torque[3], Drfl.read_data_rt()->actual_joint_torque[4], Drfl.read_data_rt()->actual_joint_torque[5]);
           printf("ext_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
                 Drfl.read_data_rt()->external_joint_torque[0], Drfl.read_data_rt()->external_joint_torque[1], Drfl.read_data_rt()->external_joint_torque[2],
                 Drfl.read_data_rt()->external_joint_torque[3], Drfl.read_data_rt()->external_joint_torque[4], Drfl.read_data_rt()->external_joint_torque[5]);
          cout <<"Robot Mode : " << Drfl.get_robot_mode() << ", Robot State : " << Drfl.get_robot_state() << ", Control Mode : " << Drfl.read_data_rt()->control_mode<<endl;


           printf("\n\n\n");
          }
					//rt_task_wait_period(NULL);
          std::this_thread::sleep_for(std::chrono::microseconds(1000*int(1000*st)));
				}
		  }
		  break;
      case 'i': // CTC
		  {
        static float time = 0.0;
				const float st=0.001; // sampling time

        float deg2rad = 3.141592653589/180;
        int count = 0;
        float q[NUMBER_OF_JOINT] = {0.0, };
        float q_dot[NUMBER_OF_JOINT] = {0.0, };
        float q_d[NUMBER_OF_JOINT] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        float q_dot_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_g[NUMBER_OF_JOINT] = {0.0, };
        float trq_d[NUMBER_OF_JOINT] = {0.0, };
        float trq_d_tmp[NUMBER_OF_JOINT] = {0.0, };
        float kp[NUMBER_OF_JOINT] = {1.0, 1.0, 1.0, 0.5, 0.5, 0.5}; // have to tune
        float kd[NUMBER_OF_JOINT] = {0.05, 0.05, 0.05, 0.025, 0.025, 0.025}; // have to tune

        // float M_mat[NUMBER_OF_JOINT][]

        float tor_ext_tmp[NUMBER_OF_JOINT] = {0.0, };
        float buf_tor_ext[NUMBER_OF_JOINT] = {0.0, };
        float tor_ext[NUMBER_OF_JOINT] = {0.0, };
        float frq_cut_off = 50; //100Hz
        float alpha = (frq_cut_off*st)/(1+frq_cut_off*st);


        float err_q[NUMBER_OF_JOINT] = {0.0,};
        float err_q_dot[NUMBER_OF_JOINT] = {0.0,};
				TraParam tra;
        int flag_trj = 0;
        int task_motion = 0;


        float vel_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};
				float acc_limit[6] = {1000, 1000, 1000, 1000, 1000, 1000};

        Drfl.set_velj_rt(vel_limit);
				Drfl.set_accj_rt(acc_limit);
        Drfl.set_velx_rt(1000,1000);
        Drfl.set_accx_rt(1000,1000);

				Drfl.set_safety_mode(SAFETY_MODE_AUTONOMOUS, SAFETY_MODE_EVENT_MOVE);


        memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);
        memcpy(buf_tor_ext, Drfl.read_data_rt()->external_joint_torque, sizeof(float)*NUMBER_OF_JOINT);

				PlanParam plan1, plan2;



				while (1)
				{
					time=(++count)*st;
          tra.time=time;

					memcpy(q, Drfl.read_data_rt()->actual_joint_position, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(q_dot, Drfl.read_data_rt()->actual_joint_velocity, sizeof(float)*NUMBER_OF_JOINT);
					memcpy(trq_g, Drfl.read_data_rt()->gravity_torque, sizeof(float)*NUMBER_OF_JOINT);
          memcpy(tor_ext_tmp, Drfl.read_data_rt()->external_joint_torque, sizeof(float)*NUMBER_OF_JOINT);


          for (int i=0; i<6; i++) {
              //trq_d[i] = (float)trq_g[i]+kp[i]*((float)tra.pos[i]-q[i])+kd[i]*((float)tra.vel[i]-q_dot[i]);

              err_q[i] = q_d[i]-q[i];
              err_q_dot[i] = q_dot_d[i]-q_dot[i];
              tor_ext[i] = alpha*tor_ext_tmp[i] + (1-alpha)*buf_tor_ext[i];
              trq_d_tmp[i] = trq_g[i]+tor_ext[i];
              // trq_d_tmp[i] = trq_g[i];
              // trq_d_tmp[i] = trq_g[i];
              // trq_d[i]=trq_g[i];

              // trq_d[i] = trq_d_tmp[i];

              // Saturation
              switch (i) {
                case 0:
                {
                  if (abs(trq_d_tmp[i]) < 244)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 244;
                }
                break;
                case 1:
                {
                  if (abs(trq_d_tmp[i]) < 244)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 244;
                }
                break;
                case 2:
                {
                  if (abs(trq_d_tmp[i]) < 143.5)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 143.5;
                }
                break;
                default:
                {
                  if (abs(trq_d_tmp[i]) < 74.5)
                    trq_d[i] = trq_d_tmp[i];
                  else
                    trq_d[i] = 74.5;
                }
                break;
              }


          }
          Drfl.torque_rt(trq_d, st);

          memcpy(buf_tor_ext, tor_ext, sizeof(float)*NUMBER_OF_JOINT);




          // cout<<"time: "<<time<<endl;
          // printf("cur_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q[0],q[1],q[2],q[3],q[4],q[5]);
          // printf("cur_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",q_dot[0],q_dot[1],q_dot[2],q_dot[3],q_dot[4],q_dot[5]);
          // printf("tar_pos: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.pos[0],tra.pos[1],tra.pos[2],tra.pos[3],tra.pos[4],tra.pos[5]);
          // printf("tar_vel: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",tra.vel[0],tra.vel[1],tra.vel[2],tra.vel[3],tra.vel[4],tra.vel[5]);
          // printf("tar_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_d_tmp[0],trq_d_tmp[1],trq_d_tmp[2],trq_d_tmp[3],trq_d_tmp[4],trq_d_tmp[5]);
          // printf("grav_tor: 1:%f, 2:%f, 3:%f, 4:%f, 5:%f, 6:%f\n",trq_g[0],trq_g[1],trq_g[2],trq_g[3],trq_g[4],trq_g[5]);
          // printf("\n\n");

          static int td = 0;
          if (td++ == 499) {
          	td = 0;
          	// printf("timestamp : %.3f\n", Drfl.read_data_rt()->time_stamp);
            printf("time : %.3f\n",time);
          	printf("joint : %f %f %f %f %f %f\n", Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2], Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
           printf("q = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_position[0], Drfl.read_data_rt()->actual_joint_position[1], Drfl.read_data_rt()->actual_joint_position[2],
           		Drfl.read_data_rt()->actual_joint_position[3], Drfl.read_data_rt()->actual_joint_position[4], Drfl.read_data_rt()->actual_joint_position[5]);
           printf("q_dot = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_velocity[0], Drfl.read_data_rt()->actual_joint_velocity[1], Drfl.read_data_rt()->actual_joint_velocity[2],
           		Drfl.read_data_rt()->actual_joint_velocity[3], Drfl.read_data_rt()->actual_joint_velocity[4], Drfl.read_data_rt()->actual_joint_velocity[5]);
           printf("trq_g = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->gravity_torque[0], Drfl.read_data_rt()->gravity_torque[1], Drfl.read_data_rt()->gravity_torque[2],
           		Drfl.read_data_rt()->gravity_torque[3], Drfl.read_data_rt()->gravity_torque[4], Drfl.read_data_rt()->gravity_torque[5]);
           printf("act_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
           		Drfl.read_data_rt()->actual_joint_torque[0], Drfl.read_data_rt()->actual_joint_torque[1], Drfl.read_data_rt()->actual_joint_torque[2],
           		Drfl.read_data_rt()->actual_joint_torque[3], Drfl.read_data_rt()->actual_joint_torque[4], Drfl.read_data_rt()->actual_joint_torque[5]);
           printf("ext_tor = %7.4f, %7.4f, %7.4f, %7.4f, %7.4f, %7.4f\n",
                 Drfl.read_data_rt()->external_joint_torque[0], Drfl.read_data_rt()->external_joint_torque[1], Drfl.read_data_rt()->external_joint_torque[2],
                 Drfl.read_data_rt()->external_joint_torque[3], Drfl.read_data_rt()->external_joint_torque[4], Drfl.read_data_rt()->external_joint_torque[5]);
          cout <<"Robot Mode : " << Drfl.get_robot_mode() << ", Robot State : " << Drfl.get_robot_state() << ", Control Mode : " << Drfl.read_data_rt()->control_mode<<endl;


           printf("\n\n\n");
          }
					//rt_task_wait_period(NULL);
          std::this_thread::sleep_for(std::chrono::microseconds(1000*int(1000*st)));
				}
		  }
		  break;
      default:
        break;
    }
    // Sleep(100);
    this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  Drfl.CloseConnection();

  return 0;
}
