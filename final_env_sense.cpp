#include "main.h"


void pico_setup();
void bme_setup();
void displaySetup();
void Test();
void acc_init();
void getBMEData();

// The BME680 data Temperature %.2f : Pressure %.2f : Humidity %.2f : Gas resistance %.2f
#define packet "%.2f:%.2f:%.2f:%.2f"

#define lora_i 3000000
#define acc_i 50000
#define sensor_i 500000

unsigned long   previousMillisLora = 0;    // will store last time was updated for Lora Transmit
unsigned long   previousMillisSensor = 0;    // will store last time was updated for Sensor data on display
unsigned long   previousMillisAcc = 0;    // will store last time was updated for Acclerometer

Lora *lora;

// Screen settings
#define myOLEDwidth  128
#define myOLEDheight 32
#define myScreenSize (myOLEDwidth * (myOLEDheight/8)) // eg 512 bytes = 128 * 32/8
uint8_t screenBuffer[myScreenSize]; // Define a buffer to cover whole screen  128 * 32/8
const uint8_t I2C_Address = 0x3C;
SSD1306 myOLED(myOLEDwidth ,myOLEDheight);

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define SCL_PIN 5
#define SDA_PIN 4

// bme stuff
struct bme68x_dev bme;
int8_t rslt;
struct bme68x_conf conf;
struct bme68x_heatr_conf heatr_conf;
struct bme68x_data data;
uint32_t del_period;
uint32_t time_ms = 0;
uint8_t n_fields;
uint16_t sample_count = 1;


void display_sensor_data() 
{
    char buf[25];
    myOLED.setCursor(0, 0);
    sprintf(buf, "Temp : - %.1f", data.temperature);
	myOLED.print(buf);
    myOLED.setCursor(70, 0);
    sprintf(buf, "Humid : - %.1f", data.humidity);
	myOLED.print(buf);
    myOLED.setCursor(0, 10);
    sprintf(buf, "Pres : - %.1f", data.pressure);
	myOLED.print(buf);
    myOLED.setCursor(0, 20);
    sprintf(buf, "GasR : - %.1f", data.gas_resistance);
	myOLED.print(buf);
	myOLED.OLEDupdate();  
    sleep_ms(100);
}


int main()
{
    pico_setup();   
    bme_setup();
    lora = new Lora();
    lora->SetTxEnable();
    sleep_ms(1000);
    lora->ProcessIrq();
    lora->CheckDeviceStatus();
    acc_init();
    displaySetup();
    while (true) {
        unsigned long currentMillis  = time_us_32();
        if ((currentMillis - previousMillisLora >= sensor_i)) {
        previousMillisSensor = currentMillis;  
        getBMEData();
        display_sensor_data();
        }
        currentMillis  = time_us_32();
        if ((currentMillis - previousMillisLora >= acc_i)) {
        previousMillisAcc = currentMillis;  
        printf("X coordinate values : %.6f \n", raw_convertor(read_accDataX()));
        printf("Y coordinate values : %.6f \n", raw_convertor(read_accDataY()));
        printf("Z coordinate values : %.6f \n", raw_convertor(read_accDataZ()));
        }
        currentMillis  = time_us_32();
        if ((currentMillis - previousMillisLora >= lora_i)) {
        previousMillisLora = currentMillis;  
        char payload[100];
        sprintf(payload, packet, data.temperature, data.pressure, data.humidity, data.gas_resistance);
        lora->SendData(22, payload, strlen(payload));
        sleep_ms(500);
        lora->ProcessIrq();
        lora->CheckDeviceStatus();
        lora->SetTxEnable();
        }
    }
}


void pico_setup(){
     stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

void bme_setup(){
    bme.intf = BME68X_I2C_INTF;
    bme.read = (bme68x_read_fptr_t)read_bytes;
    bme.write = (bme68x_write_fptr_t)write_bytes;
    bme.delay_us = (bme68x_delay_us_fptr_t)bme68x_delay_us;
    bme.amb_temp = 25;

    if (bme68x_init(&bme) != BME68X_OK) {
    char bme_msg[] = "BME680 Initialization Error\r\n";
    printf(bme_msg);
    } else {
    char bme_msg[] = "BME680 Initialized and Ready\r\n";
    printf("Driver code is connected \n");
    printf(bme_msg);
    }
        conf.filter = BME68X_FILTER_OFF;
        conf.odr = BME68X_ODR_NONE;
        conf.os_hum = BME68X_OS_16X;
        conf.os_pres = BME68X_OS_1X;
        conf.os_temp = BME68X_OS_2X;
        rslt = bme68x_set_conf(&conf, &bme);
        bme68x_check_rslt("bme68x_set_conf", rslt);
        heatr_conf.enable = BME68X_ENABLE;
        heatr_conf.heatr_temp = 300;
        heatr_conf.heatr_dur = 100;
        rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
        bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
}

void acc_init(){
    set_mode(0x00);
    printf("Set to standby mode value : %x \n", read_mode());
    printf("Set to dynamic range value : %x \n", set_dynamic_range(0x00));
    set_dynamic_range(0x00);
    set_mode(0x01);
}

void getBMEData(){
    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
    bme68x_check_rslt("bme68x_set_op_mode", rslt);

    /* Calculate delay period in microseconds */
    del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme) + (heatr_conf.heatr_dur * 1000);
    bme.delay_us(del_period, bme.intf_ptr);
    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);
    bme68x_check_rslt("bme68x_get_data", rslt);
}

void displaySetup(){
    printf("OLED SSD1306 :: Start!\r\n");

	while(myOLED.OLEDbegin(I2C_Address,I2C_PORT,  400, SDA_PIN, SCL_PIN) != true)
	{
		printf("SetupTest ERROR : Failed to initialize OLED!\r\n");
		busy_wait_ms(500);
	} // initialize the OLED
	if (myOLED.OLEDSetBufferPtr(myOLEDwidth, myOLEDheight, screenBuffer, sizeof(screenBuffer)/sizeof(uint8_t)) != 0)
	{
		printf("SetupTest : ERROR : OLEDSetBufferPtr Failed!\r\n");
		while(1){busy_wait_ms(1000);}
	} // Initialize the buffer
	myOLED.OLEDFillScreen(0xF0, 0); // splash screen bars

    myOLED.OLEDclearBuffer(); 
	myOLED.setFont(pFontPico);
    myOLED.setCursor(0, 0);
	myOLED.print("Connect :)");
	myOLED.OLEDupdate();  
    sleep_ms(1000);
    myOLED.OLEDclearBuffer(); 
    myOLED.OLEDupdate(); 
}

