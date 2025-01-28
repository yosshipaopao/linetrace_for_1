#include <ESP32Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// B
#define LEFT_1 4
#define LEFT_2 2
// analog
#define LEFT_A 15
// channnel
#define LEFT_C 2
// A
#define RIGHT_1 16
#define RIGHT_2 17
// analog
#define RIGHT_A 5 
// channnel
#define RIGHT_C 3 

#define PanjandrumLPIN 19
#define PanjandrumRPIN 18

#define Switch_PIN 12
//program config 

#define Half_trun_delay 1400

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG  

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

#include <RemoteXY.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "panjandrum"


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 89 bytes
  { 255,5,0,0,0,82,0,16,250,1,4,48,0,0,13,100,2,26,4,48,
  50,0,13,100,2,26,2,1,21,1,22,11,2,26,31,31,79,78,0,79,
  70,70,0,2,1,21,88,22,11,2,26,31,31,65,85,84,79,0,77,65,
  78,85,65,76,0,2,1,21,46,22,11,2,26,31,31,80,65,83,83,69,
  68,0,66,69,70,79,82,69,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t LEFT; // =-100..100 slider position 
  int8_t RIGHT; // =-100..100 slider position 
  uint8_t POWER; // =1 if switch ON and =0 if OFF 
  uint8_t MODE; // =1 if switch ON and =0 if OFF 
  uint8_t AFTER_AUTO; // =1 if switch ON and =0 if OFF 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////



// set 関数で値を読み取ります
// data[i]  で直接値(int)を取得できます。 
// get(i)で黒色かどうか(bool)を獲得できます。
// any_middle()で真ん中4つどれかでセンサーがついてるか(bool)
class Sensor{
  private:
    // pinの数
    int pinsize = 6;
    // pin番号
    int pins[6] = {
      33,
      32,
      35,
      34,
      39,
      36
    };
    // pinの判断基準
    int bases[6] = {
      2000,
      1900,
      2000,
      2000,
      2000,
      2000
    };
  public:
    // 読み込んだデータの一時保存場所
    int data[6];
    // setup
    void setup(){
      for(int i=0; i < pinsize; i++){
        pinMode(pins[i],ANALOG);
      }
    };
    //　値を読んで一時保存場所にセット
    void set(){      
      for(int i=0;i<pinsize;i++)data[i]=analogRead(pins[i]);
    }
    // 値の取得(基準を超えてるかチェック)
    bool get(int i){
      return data[i] > bases[i];
    };
    // Serialで値（上位二けたを表示)
    void debug(){
      for(int i=0; i < pinsize; i++){
        Serial.print(data[i]/100);
        Serial.write(" ");
      }
      Serial.write("\n");
    };
};

// speed は -255 ~ 255で設定できます。
// straight(speed) で直進（または後退）
// turn(left_speed,right_speed) で回転
class Motor{
  private:
    // 左のモーター用の制御(諸説あり)
    void leftMotor(int speed) {  // -255 ~ 255
      if (speed > 0) {
        digitalWrite(LEFT_1, HIGH);
        digitalWrite(LEFT_2, LOW);
        ledcWrite(LEFT_C, speed);
      } else {
        digitalWrite(LEFT_1, LOW);
        digitalWrite(LEFT_2, HIGH);
        ledcWrite(LEFT_C, speed * -1);
      }
    };
    // 右のモーター用の制御(諸説あり)
    void rightMotor(int speed) {  // -255 ~ 255
      if (speed > 0) {
        digitalWrite(RIGHT_1, HIGH);
        digitalWrite(RIGHT_2, LOW);
        ledcWrite(RIGHT_C, speed);
      } else {
        digitalWrite(RIGHT_1, LOW);
        digitalWrite(RIGHT_2, HIGH);
        ledcWrite(RIGHT_C, speed * -1);
      }
    };
  public:
    // setup
    void setup(){
      // left
      ledcSetup(LEFT_C, 12800, 8);
      ledcAttachPin(LEFT_A, LEFT_C);
      pinMode(LEFT_1, OUTPUT);
      pinMode(LEFT_2, OUTPUT);
      // right
      ledcSetup(RIGHT_C, 12800, 8);
      ledcAttachPin(RIGHT_A, RIGHT_C);
      pinMode(RIGHT_1, OUTPUT);
      pinMode(RIGHT_2, OUTPUT);
    };
    void straight(int speed) {
      leftMotor(speed);
      rightMotor(speed);
    };
    void turn(int left_speed, int right_speed) {
      leftMotor(left_speed);
      rightMotor(right_speed);
    };
};

// speedは -90 ~ 90
// straight(speed) で直進（または後退）
// turn(left_speed,right_speed) で回転
class Panjandrum{
  private:
    // ESP32Servo の ラッパー
    Servo PanjandrumL;
    Servo PanjandrumR;
  public:
    void setup(){
      PanjandrumL.attach(PanjandrumLPIN, 500, 2400);
      PanjandrumR.attach(PanjandrumRPIN, 500, 2400);
      PanjandrumL.write(92);
      PanjandrumR.write(90);
    };
    // 直進処理（いらなくね？って突っ込みはなしで(可読性のため)）
    void straight(int speed) {
      PanjandrumL.write(-speed+92);
      PanjandrumR.write(speed+90);
    };
    // 回転(左のモーターと右のモーターの速度をそれぞれ)
    void turn(int left_speed, int right_speed) {
      PanjandrumL.write(-left_speed+92);
      PanjandrumR.write(right_speed+90);
    };
};

Motor motor;
Sensor sensor;
Panjandrum panjandrum;

SoftwareSerial mySoftwareSerial(26, 27); // IO10をRX, IO11をTXとしてアサイン
DFRobotDFPlayerMini myDFPlayer;


// 直進用
// spped -100 ~ 100
void straight(int speed){
  motor.straight( speed * 255 / 100 );
  panjandrum.straight( speed * 70 / 100 );
}
// 回転用、 第一引数でl側、第二引数でr側を設定できます。
// spped -100 ~ 100
void turn(int l_speed,int r_speed){
  motor.turn( l_speed * 255 / 100 , r_speed * 255 / 100 );
  panjandrum.turn( l_speed * 70 / 100 , r_speed * 70 / 100 );
}


// 通常走行の速度
int RUN_SP=66;
// ちょっとした曲がり用の速度
int HALF_STOP_SP=40;
// ちゃんと曲がる用の速度
int STOP_SP=25;

void setup() {
  RemoteXY_Init (); 
  // main motor
  motor.setup();
  // sensor
  sensor.setup();
  // Panjandrum
  panjandrum.setup();
  // switch
  pinMode( Switch_PIN, INPUT_PULLUP );
  // DFPlayer
  mySoftwareSerial.begin(9600);
  Serial.begin(4800);
  // DFPlayerのチェック
  if(!myDFPlayer.begin(mySoftwareSerial)){
    Serial.write("failed to connect DFPlayer\n");
  }
  // Sync
  RemoteXY_delay(1000);
  // DFPlayerの音量設定
  myDFPlayer.volume(30);
}


// timer(10msごとに1++)
unsigned long long timer = 0LL;
// 音タイマーーーーーーー
// -1の時流さない
// 0~3500で今どのくらい再生されているか
int Sound_timer = -1;

// 前回どっち側検出したか
// 1で左？-1で右？
int before = 0;

// 一番端が最後に検出されたタイミング
int l_detected = 0;
// 何回端が検出されたか
int detect_cnt=0;
// Cに入っているかどうか(0以上で入ってる)
int mode_c= -1;

// remoteでbefore か passedがのフラグでへんこうされたかチェックするぞ
int before_remote_after=0;

// スイッチ何秒カウント
int Switch_cnt=0;
// なんも検出されないのが何秒続いたかやで
int not_cnt=0;

void loop() {
  RemoteXY_Handler();
  // beforeとpassedが切り替わったら初期化
  if(before_remote_after!=RemoteXY.AFTER_AUTO){
    detect_cnt=0;
    before_remote_after=RemoteXY.AFTER_AUTO;
  }
  // スイッチがついてたら強制停止
  if(digitalRead(Switch_PIN)==0){
    straight(0);
    myDFPlayer.stop();
    Sound_timer=-1;
    Switch_cnt++;
    if(Switch_cnt>5){
      detect_cnt=0;
      Switch_cnt=0;
    }
    RemoteXY_delay(100);
    return ;
  }else Switch_cnt=0;
  // power is off or not connected -> do not run
  if(RemoteXY.connect_flag==0||RemoteXY.POWER==0) straight(0);
  else if(RemoteXY.MODE){ // ライントレースモード
    sensor.set(); //　センサーの値を読み取ります。
    ///// line trace start
    // 真ん中二つが同時についたら直進するやで
    if(sensor.get(2) && sensor.get(3)){
      straight(RUN_SP);
      before=0;
    }
    // 左だけだったら左に
    else if(sensor.get(2)){
      turn(RUN_SP,HALF_STOP_SP);
      before=0;
      not_cnt=0;
    }
    // 右だけだったら左に
    else if(sensor.get(3)){
      turn(HALF_STOP_SP,RUN_SP);
      before=0;
      not_cnt=0;
    }
    // 左の真ん中だったら強く曲がるとflagを設置
    else if(sensor.get(1)){
      turn(RUN_SP,STOP_SP);
      before=1;
    }
    // 右の真ん中だったら強く曲がるとflagを設置
    else if(sensor.get(4)) {
      turn(STOP_SP,RUN_SP);
      before=-1;
    }
    // 真ん中が検出されてないとき
    else{
      if(mode_c>=0 && not_cnt>30){// C到達時の180度ターン
        mode_c=-1;
        straight(-50);
        RemoteXY_delay(1000);
        turn(RUN_SP,-RUN_SP);
        RemoteXY_delay(Half_trun_delay);
      }else if(before>0){
        // 前回左の時
        turn(RUN_SP,STOP_SP);
        not_cnt++;
      }else if(before<0){
        // 前回右の時
        turn(STOP_SP,RUN_SP);
        not_cnt++;
      }else {
        // どっちでもないとき
        straight(50);
        not_cnt++;
      }
    }
    ///// line trace end
    // ここelse ifじゃない！！！！！！！
    if(sensor.get(0)){// 左のセンサーが出てた時
      if(timer-l_detected>100){//検出から一秒以内は再度実行しない
        l_detected=timer;
        detect_cnt++;// 回数を追加
        if(RemoteXY.AFTER_AUTO==0){// beforeの時
          //　二回までは無視
          if(detect_cnt>=2){
            // 二回より後は一時停止
            straight(0);
            RemoteXY_delay(3000);//3秒
          }
          Sound_timer=-1;// 音は停止
        }else{// passed
          if(detect_cnt==1)Sound_timer=0;// 一回目は音を流す
          else if(detect_cnt==2||detect_cnt==3){// ２，３回目は90回転
            if(mode_c==-1){
              straight(-50);
              RemoteXY_delay(400);
              turn(RUN_SP,0);
              RemoteXY_delay(Half_trun_delay);
              if(detect_cnt==2)mode_c=timer;
            }else detect_cnt--;
          }
          //　それ以降は無視
        }
      }
    }
  } else { // 手動操作
    /* for sensor debugging
    */sensor.set();// set values  
    sensor.debug();
    /**/
    turn(RemoteXY.RIGHT,RemoteXY.LEFT);
  }
  //気合の音楽再生
  if(Sound_timer>=0){
    if(Sound_timer>3150)Sound_timer=0;
    if(Sound_timer==0)myDFPlayer.play(1);
    Sound_timer++;
  }else{
    myDFPlayer.stop();
  }
  // タイマー処理
  timer++;
  RemoteXY_delay(10);
}