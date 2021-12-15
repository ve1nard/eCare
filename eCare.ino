  /*declaring all the necessary libraries:*/
  #include <M5Core2.h>
  #include <Arduino.h>
  #if defined(ESP32)
  #include <WiFi.h>//this libraryconnects the device to Wi-Fi
  #elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #endif
  #include <ESP_Mail_Client.h>
  #define WIFI_SSID "A1.C724"//Wi-Fi name
  #define WIFI_PASSWORD "A1C72412"//Wi-Fi password
  #define SMTP_HOST "smtp.gmail.com"//for SMTP protocol
  #define SMTP_PORT 465
  /* The sign in credentials */
  #define AUTHOR_EMAIL "mstackecare@gmail.com"//M5Stacks's email
  #define AUTHOR_PASSWORD "1234ecare"//The email's password  
  /* Recipient's email*/
  #define RECIPIENT_EMAIL "zshegenov@gmail.com"  
  /* The SMTP Session object used for Email sending */
  SMTPSession smtp;
  /* Callback function to get the Email sending status */
  void smtpCallback(SMTP_Status status);

  /*necessary constants:*/
  ButtonColors on_clrs = {GREEN, WHITE, WHITE};
  ButtonColors off_clrs = {BLUE, WHITE, WHITE};
  Button intakeReminder(0, 0, 0, 0, false ,"Intake reminder", off_clrs, on_clrs, MC_DATUM);
  Button stepCounter(0, 0, 0, 0, false, "Step counter", off_clrs, on_clrs, MC_DATUM);
  Button notifications(0, 0, 0, 0, false, "Notifications", off_clrs, on_clrs, MC_DATUM);
  Button userManual(0, 0, 0, 0, false, "User manual", off_clrs, on_clrs, MC_DATUM);   
  bool intakeReminderOpened = false;
  bool stepCounterOpened = false;
  bool notificationsOpened = false;
  bool manualOpened = false;
  bool isTimeSet = false;
  bool dailyStepCountIsReached = false;
  bool newNotifications = false;
  bool checkTime = false;
  bool startedSetting = false;
  int stepCount = 0;
  int hours=0;
  int minutes=0;
  int hoursNextIntake = 0;
  int minutesNextIntake = 0;
  float total = 0;
  int count = 0;
  float avg = 1.1;
  float width = avg / 10;
  boolean state = false;
  boolean old_state = false;
  bool mainMenuPrinted = false;
  bool clearNotification = false;
  float accX = 0.0F;  // Define variables for storing inertial sensor data
  float accY = 0.0F; 
  float accZ = 0.0F;  
  float avAcc = 0.0F;
  int minutesFall = 0;
  bool getTime = false;
  bool checkTimeFall = false;
  bool msgSent = false;
  bool isNotSafe = false;
  

RTC_TimeTypeDef TimeStruct;//used for time counting & TimeStruct is for retrieving hour, minute, and seconds values
  
void setup()
{
  M5.begin();//sets the program
  M5.Axp.SetLDOEnable(3,false);//disables vibration motor
  M5.IMU.Init();//for retrieving an acceleration of the device and initialisation
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);//connect to W-Fi
  printMainMenu();//printing main menu
}

void loop() {
  M5.update();
  M5.Rtc.GetTime(&TimeStruct);//constantly retrieving time 
  if (intakeReminderOpened)//if the user opened the intake reminder option
  {
    if (isTimeSet)//if the time was set before then the user is notified how many time they have left
    {
      //colour and text adjustments:
      M5.Lcd.setTextColor(BLACK, WHITE); 
      M5.Lcd.setCursor(70,20);
      M5.Lcd.printf("Time left till the next the intake: \n");
      M5.Lcd.setCursor(50,140);
      M5.Lcd.setTextSize(4);

      /*Below operations are made for intake reminder*/
      if (TimeStruct.Hours < hours)
      {
        int tempHours = hours;//storing hour value set by the user
        hoursNextIntake = tempHours - TimeStruct.Hours;//how many hours are left to intake
        int tempMinutes;
        /*Anticipating some cases with timing: hour & minute variables are adjusted for different cases*/
        if (minutes == 0)//e.g. when minutes are set to 0
        {
          tempMinutes = 60;
          if (hoursNextIntake == 1)
          {
            hoursNextIntake = 0;
          }
        }
        else
        {
          tempMinutes = minutes;
        }        
        minutesNextIntake = tempMinutes - TimeStruct.Minutes;
      }
      else
      {
        hoursNextIntake = 24 - TimeStruct.Hours + hours;
        minutesNextIntake = TimeStruct.Minutes - minutes;
      }     
      M5.Lcd.printf("%02d : ", hoursNextIntake);   
      M5.Lcd.printf("%02d \n", minutesNextIntake); 
      M5.Lcd.setTextSize(1);
      String str = "<<<";
      M5.Lcd.drawString(str, 60, 230, 4); 
      if (M5.BtnA.isPressed())//if the first button was pressed, the main menu is shown
      {
      printMainMenu();
      intakeReminderOpened = false;//disables the main menu printing 
      }
    }
    else
    {
      if (!startedSetting)//if the timing was not set then 00:00 is printed
      {
        M5.Lcd.setCursor(50,140);
        M5.Lcd.setTextSize(4);
        M5.Lcd.printf("00 : 00");
      }
      /*prompts the user to select the timing*/
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(BLACK, WHITE); 
      M5.Lcd.setCursor(90,20);
      M5.Lcd.printf("Select the timing: \n");          
      String str = "Hours";
      M5.Lcd.drawString(str, 50, 230, 4);
      str = "Minutes";
      M5.Lcd.drawString(str, 160, 230, 4);
      str = "Set";
      M5.Lcd.drawString(str, 265, 230, 4);
      /*Button A is used for setting hours*/
      if (M5.BtnA.wasPressed()) 
      {
        startedSetting = true;
        hours++;
        if (hours>23)
        {
          hours=0;
        } 
        M5.Lcd.clear();
        M5.Lcd.fillRect(0,0, 320, 240, WHITE);
        M5.Lcd.setTextColor(BLACK, WHITE); 
        M5.Lcd.setCursor(90,20);
        M5.Lcd.printf("Select the timing: \n");
        M5.Lcd.setCursor(50,140);
        M5.Lcd.setTextSize(4);
        M5.Lcd.printf("%02d : ", hours);   
        M5.Lcd.printf("%02d \n", minutes); 
        M5.Lcd.setTextSize(1);    
      }
      /*Button B is for setting the minutes*/
      if (M5.BtnB.wasPressed()) 
      {
        startedSetting = true;
        minutes++;
        if (minutes>59)
        {
          minutes=0;
        } 
        M5.Lcd.clear();
        M5.Lcd.fillRect(0,0, 320, 240, WHITE);
        M5.Lcd.setTextColor(BLACK, WHITE); 
        M5.Lcd.setCursor(90,20);
        M5.Lcd.printf("Select the timing: \n");
        M5.Lcd.setCursor(50,140);
        M5.Lcd.setTextSize(4);
        M5.Lcd.printf("%02d : ", hours);   
        M5.Lcd.printf("%02d \n", minutes); 
        M5.Lcd.setTextSize(1);    
      }
      /*Button C sets the time and exits back to the main menu*/
      if (M5.BtnC.wasPressed())
      {
        M5.Lcd.clear();
        M5.Lcd.setCursor(0,0);
        checkTime=true;  
        isTimeSet = true;
        printMainMenu();
        intakeReminderOpened = false;
        startedSetting = false;
      }      
    }
  }
  if (checkTime)// if time has passed, then vibration motor starts working for 1 minute
  {
    if (TimeStruct.Hours==hours && TimeStruct.Minutes==minutes)
    {
      M5.Axp.SetLDOEnable(3,true);
    }
    if (TimeStruct.Hours==hours && TimeStruct.Minutes==minutes+1)
    {
      M5.Axp.SetLDOEnable(3,false);
      
    }
  }
  if (stepCounterOpened)// if the user opens the step-counter, then the no. of steps is displayed
  {
    if (M5.BtnA.wasPressed())//Button A exits back to the main menu
    {
      printMainMenu();
      stepCounterOpened = false;
    }
  }
  if (stepCount >= 5000)//if the no. of steps exceeds the limit, then the user is notified
  {
    dailyStepCountIsReached = true;
  }
  if (dailyStepCountIsReached)//1 notification appears
  {
    newNotifications = true;
    if (mainMenuPrinted == true)
    {
      printMainMenu();
      mainMenuPrinted = false;
    }
  }
  else
  {
    newNotifications = false;//if the notification is read
  }
  if (notificationsOpened)
  { 
    newNotifications = false;
    if (M5.BtnA.isPressed())
    {
      printMainMenu();
      notificationsOpened = false;
      dailyStepCountIsReached = false;
      clearNotification = true;
    }
  }
  if (manualOpened)//opening the user's manual
  {
    if (M5.BtnA.isPressed())//exiting back to the main menu
    {
      printMainMenu();
      manualOpened = false;
    }
  }
  /*Below variables are initialised for acceleration in three dimensions*/
  float accX = 0;
  float accY = 0;
  float accZ = 0;

  M5.IMU.getAccelData(&accX, &accY, &accZ);//get data from acceleration and store them into variables
  float accel = sqrt(accX * accX + accY * accY + accZ * accZ);//net acceleration

  // Calibration for average acceleration.
  if (count < 1000) {
    total += accel;
    count += 1;
  } else {
    avg = total / count;
    width = avg / 10;
    total = avg;
    count = 1;
  }

  // When current acceleration is ...
  if (accel > avg + width) {
    state = true;
  } else if (accel < avg - width) {
    state = false;
  }

  // Count steps.
  if (!old_state && state) {
    stepCount += 1;
    // Display step
  } else {

  }
  old_state = state;

  delay(50);
  
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  if (accZ>1.9)
  {
    getTime=true;//set true for gettime
  }
  if (getTime)//start counting the time
  /*This case anticipates fall of the device without the user*/
  {
    M5.update();
    RTC_TimeTypeDef TimeStruct2;
    M5.Rtc.GetTime(&TimeStruct2);
    minutesFall = TimeStruct2.Minutes;
    getTime=false;
    checkTimeFall=true;   
  }
  if (checkTimeFall)
  {
    RTC_TimeTypeDef TimeStruct2;
    M5.Rtc.GetTime(&TimeStruct2); 
    printSafetyScrean();
    isNotSafe = true;
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed())
    {
      /*if any button was pressed, then detection signal won't be sent*/
      isNotSafe = false;
      checkTimeFall=false; 
      printMainMenu();
    }
    if (TimeStruct2.Minutes==minutesFall+1 && isNotSafe)
    { 
      /*//when 1 minute has passed, an email is sent*/
      M5.IMU.getAccelData(&accX,&accY,&accZ);
      avAcc=sqrt(accX*accX+accY*accY+accZ*accZ);
      if(avAcc<2 && !msgSent)
      { 
        smtp.debug(1);        
        /* Declare the session config data */
        ESP_Mail_Session session;      
        /* Set the session config */
        session.server.host_name = SMTP_HOST;
        session.server.port = SMTP_PORT;
        session.login.email = AUTHOR_EMAIL;
        session.login.password = AUTHOR_PASSWORD;
        session.login.user_domain = "";      
        /* Declare the message class */
        SMTP_Message message;      
        /* Set the message headers */
        message.sender.name = "eCare";
        message.sender.email = AUTHOR_EMAIL;
        message.subject = "Potential fall detection";
        message.addRecipient("Dear Customer", RECIPIENT_EMAIL);      
        
        //Send raw text message
        String textMsg = "Potential fall have been detected";
        message.text.content = textMsg.c_str();
        message.text.charSet = "us-ascii";
        message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;        
        message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
        message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;      
        /* Set the custom message header */
        //message.addHeader("Message-ID: <abcde.fghij@gmail.com>");      
        /* Connect to server with the session config */
        if (!smtp.connect(&session))
          return;      
        /* Start sending Email and close the session */        
        MailClient.sendMail(&smtp, &message);
        msgSent = true;
          
       }
    }
  } 
}
void intakeBtnTap(Event& e)
/*This function returns true for intakeReminderOpened if the LCD button for Intake is pressed*/
{
  intakeReminderOpened = true;
  M5.Lcd.clear(); 
  M5.Lcd.fillRect(0,0, 320, 240, WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE); 
}
void counterBtnTap(Event& e) 
{
  /*When the LCD-button for step-counting is pressed, displays the number of steps walked */
  stepCounterOpened = true;
  ButtonColors on_clrs = {GREEN, WHITE, WHITE};
  ButtonColors off_clrs = {BLUE, WHITE, WHITE};
  M5.Lcd.clear(); 
  M5.Lcd.fillRect(0,0, 320, 240, WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setCursor(0, 20);
  String str = "You step count is ";
  M5.Lcd.drawString(str, 160, 20, 4);
  M5.Lcd.setCursor(125, 140);
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("%02d",stepCount);
  M5.Lcd.setTextSize(1);
  str = "<<<";
  M5.Lcd.drawString(str, 60, 230, 4);  
}
void notificationsBtnTap(Event& e)
/*This function is for notifying the user about reaching the daily norm of walking*/
{
  notificationsOpened = true;
  ButtonColors on_clrs = {GREEN, WHITE, WHITE};
  ButtonColors off_clrs = {BLUE, WHITE, WHITE};
  M5.Lcd.clear(); 
  M5.Lcd.fillRect(0,0, 320, 240, WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  if (dailyStepCountIsReached)
  {
    M5.Lcd.clear(); 
    M5.Lcd.fillRect(0,0, 320, 240, WHITE);
    M5.Lcd.setTextColor(BLACK, WHITE);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf("Congratulations! The daily step norm of steps is reached! You step count is %02d. Keep it up! \n", stepCount );
    M5.Lcd.setTextSize(1);
    String str = "<<<";
    M5.Lcd.drawString(str, 60, 230, 4); 
  }
  else
  {
    M5.Lcd.clear(); 
    M5.Lcd.fillRect(0,0, 320, 240, WHITE);
    M5.Lcd.setTextColor(BLACK, WHITE);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(1);
    String str = "<<<";
    M5.Lcd.drawString(str, 60, 230, 4);  
  }

}
void manualBtnTap(Event& e) 
/*This function displays User Manual/Guide*/
{
  manualOpened = true;
  ButtonColors on_clrs = {GREEN, WHITE, WHITE};
  ButtonColors off_clrs = {BLUE, WHITE, WHITE};
  M5.Lcd.clear(); 
  M5.Lcd.fillRect(0,0, 320, 240, WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Intake reminder: set the timing using\n");
  M5.Lcd.printf("'Hour' and 'Minute' buttons.\n"); 
  M5.Lcd.printf("Confirm your choice with 'Set' button \n");
  M5.Lcd.printf("Notifications: you can see how many\n");
  M5.Lcd.printf("steps you have done and whether you\n");  
  M5.Lcd.printf("have reached the daily norm \n");
  M5.Lcd.printf("In case if the fall is detected, an email is sent to a given address \n");
  String str = "<<<";
  M5.Lcd.drawString(str, 60, 230, 4);  
}

void printMainMenu()
{
  mainMenuPrinted = true; //sets the state of the menu
  M5.Lcd.clearDisplay(); //clearing the screen
  M5.Lcd.fillScreen(WHITE); //fill screen with white colout
  int16_t hw = M5.Lcd.width() / 2; 
  int16_t hh = M5.Lcd.height() / 2;
  /*setting the main menu configurations*/
  intakeReminder.set(0, 0, hw - 5, hh - 5);
  stepCounter.set(0, hh + 5, hw - 5, hh - 5);
  notifications.set(hw + 5, 0, hw - 5, hh - 5);
  //if there are some notifications present, then the section is coloured into Red
  if (newNotifications && !clearNotification)
  {
    notifications.off.bg = RED;
  }
  else //if there are none, then the colour is changed/remains to blue
  {
    notifications.off.bg = BLUE;
  }
  /*functions for LCD-buttons interactions*/
  userManual.set(hw + 5, hh + 5, hw - 5, hh - 5);
  intakeReminder.addHandler(intakeBtnTap, E_TAP);
  stepCounter.addHandler(counterBtnTap, E_TAP);
  notifications.addHandler(notificationsBtnTap, E_TAP);
  userManual.addHandler(manualBtnTap, E_TAP);
  M5.Buttons.draw();//displays LCD-buttons
  
}
void printSafetyScrean()//prints a message for fall detection
{
  M5.Lcd.fillRect(0,0, 320, 240, WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Fall has been detected. Please, press any button to confirm that you are safe");
}
