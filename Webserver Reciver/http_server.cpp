/*
    Make a simple on-the-fly generated web page using my WebServer library on top of Lightweight IP (LwIP).

    Ted Rossin  5-26-2023
                7-02-2023
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "lib/WebServer.h"
#include "lib/WifiInfo.h"  // Defines WIFI_SSID and WIFI_PASSWORD
#include "sx126x/lora.h"
#include "sx126x/sx126x_hal.h"
#include "hardware/spi.h"

WebServer Ws;

static struct{
    int SendCount;
    float temperature;
} State;

extern int flag;
extern SensorData data;

void ProcessRequest(const char *Req)
{
    char *Ptr,Info[100];
    int i;

    strncpy(Info,Req,100);
    Ptr = strstr(Info,"\n");  if(Ptr) *Ptr = 0;  else Info[40] = 0;
    printf("    Got Request: %s\n",Info);
    State.SendCount++;
    Ws.SendText(
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "\n" //  This is needed (for Chrome, Edge and Safari).  I don't know why.
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<body>\n"
        "<div style=\"padding:20px; border-radius:15px; background:linear-gradient(135deg, #ffcccc, #ffe6cc); box-shadow:0 4px 12px rgba(0, 0, 0, 0.3);\">\n"
        "<h1 style=\"font-size:2em; color:#4a4a4a; margin-bottom:20px; text-shadow:1px 1px 4px rgba(0, 0, 0, 0.2);\">Env-Sense</h1>\n"
        "<div style=\"margin:15px 0; font-size:1.4em; padding:15px; background:#d1f7d6; border-radius:8px; box-shadow:0 2px 4px rgba(0, 0, 0, 0.2);\">Temperature: <span>25&deg;C</span></div>\n"
        "<div style=\"margin:15px 0; font-size:1.4em; padding:15px; background:#d1f7d6; border-radius:8px; box-shadow:0 2px 4px rgba(0, 0, 0, 0.2);\">Humidity: <span>60%</span></div>\n"
        "<div style=\"margin:15px 0; font-size:1.4em; padding:15px; background:#d1f7d6; border-radius:8px; box-shadow:0 2px 4px rgba(0, 0, 0, 0.2);\">Gas: <span>Moderate</span></div>\n"
        "<div style=\"margin:15px 0; font-size:1.4em; padding:15px; background:#d1f7d6; border-radius:8px; box-shadow:0 2px 4px rgba(0, 0, 0, 0.2);\">Pressure: <span>1013 hPa</span></div>\n"
        "</div>\n");
    Ws.SendText("<a href=\"https://www.youtube.com/watch?v=dQw4w9WgXcQ\">Totally unsuspious link</a><br>");   
        Ws.Printf("<br>Temperature : %f",State.temperature);
    Ws.SendText("</body>\n"
               "</html>\n"); 
}

Lora *lora;

int main() 
{  
    State.SendCount = 0;

    stdio_init_all();
    printf("[Main] Setting up Lora Chip");
    lora = new Lora();
    lora->SetRxEnable();
    printf("[Main] Done");
    printf("Failed to connect\n");
    if(Ws.Init(WIFI_SSID,WIFI_PASSWORD)){
        printf("Failed to connect\n");  while(1);  // Hang
    }
    printf("Connected to %s\n",Ws.FetchAddr());
        // Handle messages from web server.  Will not return. (IdleSleepTime_ms,RequestCallback)
    while(1){
        WebMsg_t Msg;
        Msg = Ws.ReadMessage();
        lora->SetToReceiveMode();
        if(Msg.Type == 1){
        Ws.SendText(
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "\n" //  This is needed (for Chrome, Edge and Safari).  I don't know why.
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<meta http-equiv=\"refresh\" content=\"30\">\n"
        "</head>\n"
        "<body style=\"font-family:'Arial', sans-serif; margin:0; padding:0; background-color:#e8f5e9; display:flex; justify-content:center; align-items:center; min-height:100vh;\">\n");
        Ws.Printf("<div style=\"width:90%; max-width:1200px; text-align:center; display:flex; flex-direction:column; gap:20px;\">\n");
        Ws.Printf("<h1  style=\"font-size:3em; color:#1a73e8; margin-bottom:30px; text-shadow:2px 2px 5px rgba(0, 0, 0, 0.2);\">Env-Sense Dashboard</h1>\n");
        Ws.Printf("<div style=\"display:flex; flex-wrap:wrap; justify-content:center; gap:20px;\">\n");
        Ws.Printf("<div style=\"flex:1 1 45%; max-width:45%; min-width:250px; font-size:1.4em; padding:20px; background:#ffcccb; border-radius:10px; box-shadow:0 4px 6px rgba(0, 0, 0, 0.2);\">Temperature: <span style=\"font-weight:bold; color:#b22222;\">%.1f&deg;C</span></div>\n",data.temperature);
        Ws.Printf("<div style=\"flex:1 1 45%; max-width:45%; min-width:250px; font-size:1.4em; padding:20px; background:#add8e6; border-radius:10px; box-shadow:0 4px 6px rgba(0, 0, 0, 0.2);\">Pressure: <span style=\"font-weight:bold; color:#00008b;\">%.1f</span></div>\n",data.humidity);
        Ws.Printf("<div style=\"flex:1 1 45%; max-width:45%; min-width:250px; font-size:1.4em; padding:20px; background:#90ee90; border-radius:10px; box-shadow:0 4px 6px rgba(0, 0, 0, 0.2);\">Humidity: <span style=\"font-weight:bold; color:#006400;\">%.1f</span></div>\n",data.pressure);
        Ws.Printf("<div style=\"flex:1 1 45%; max-width:45%; min-width:250px; font-size:1.4em; padding:20px; background:#f0e68c; border-radius:10px; box-shadow:0 4px 6px rgba(0, 0, 0, 0.2);\">Gas: <span style=\"font-weight:bold; color:#8b4513;\">%.1f</span></div>\n",data.gasResistance);
        Ws.Printf("</div>\n");
        Ws.Printf("</div>\n");
     // Ws.SendText("<br><a href=\"https://www.youtube.com/watch?v=dQw4w9WgXcQ\">Totally unsuspious link</a>");
        Ws.SendText("</body>\n"
                    "</html>\n"); 
            }
    lora->ProcessIrq();
    lora->CheckDeviceStatus();
    sleep_ms(50);
    }
  // Ws.ProcessMessages(50,ProcessRequest);
        // Never get here
    return 0;
}

        // "<div style=\"margin:15px 0; font-size:1.4em; padding:15px; background:#d1f7d6; border-radius:8px; box-shadow:0 2px 4px rgba(0, 0, 0, 0.2);\">Humidity: <span>60%</span></div>\n"
        // "<div style=\"margin:15px 0; font-size:1.4em; padding:15px; background:#d1f7d6; border-radius:8px; box-shadow:0 2px 4px rgba(0, 0, 0, 0.2);\">Gas: <span>Moderate</span></div>\n"
        // "<div style=\"margin:15px 0; font-size:1.4em; padding:15px; background:#d1f7d6; border-radius:8px; box-shadow:0 2px 4px rgba(0, 0, 0, 0.2);\">Pressure: <span>1013 hPa</span></div>\n"