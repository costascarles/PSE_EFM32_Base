#include "my_i2c.h"
#include "em_i2c.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "semphr.h"


#define ACK_CHECK_EN 0x1 //!< I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0 //!< I2C master will not check ack from slave

static SemaphoreHandle_t i2c_semaphore = NULL;
static uint8_t device_addr;
int _write(int file, const char *ptr, int len) {
    int x;
    for (x = 0; x < len; x++) {
       ITM_SendChar (*ptr++);
    }
    return (len);
}

void my_i2c_init(uint8_t addr)
{
    // Don't initialize twice
    if(i2c_semaphore == NULL)
    	i2c_semaphore = xSemaphoreCreateBinary();

    if(i2c_semaphore == NULL){
    	printf("Error Creating Semaphore");
    }else{
    	xSemaphoreTake(i2c_semaphore,0);
    	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
		CMU_ClockEnable(cmuClock_I2C1, true);
		GPIO_PinModeSet(gpioPortC, 4, gpioModeWiredAnd, 1);
		GPIO_PinModeSet(gpioPortC, 5, gpioModeWiredAnd, 1);
		I2C1->ROUTE = I2C_ROUTE_SDAPEN |
		I2C_ROUTE_SCLPEN | I2C_ROUTE_LOCATION_LOC0;
		I2C_Init(I2C1, &i2cInit);
		device_addr = addr;
		xSemaphoreGive(i2c_semaphore);
    }



}

bool my_i2c_write(uint8_t reg, uint8_t data)
{
	xSemaphoreTake(i2c_semaphore,0);
	I2C_TransferReturn_TypeDef I2C_Status;
		bool ret_value = false;

		I2C_TransferSeq_TypeDef seq;
		uint8_t dataW[2];

		seq.addr = device_addr;
		seq.flags = I2C_FLAG_WRITE;

		/* Register to write: 0x67 ( INT_FLAT )*/
		dataW[0] = reg;
		dataW[1] = data;

		seq.buf[0].data = dataW;
		seq.buf[0].len = 2;
		I2C_Status = I2C_TransferInit(I2C1, &seq);

		while (I2C_Status == i2cTransferInProgress) {
			I2C_Status = I2C_Transfer(I2C1);
		}

		if (I2C_Status != i2cTransferDone) {
			ret_value = false;
		} else {
			ret_value = true;
		}

	xSemaphoreGive(i2c_semaphore);
	return ret_value;
}

bool my_i2c_read(uint8_t reg, uint8_t *val)
{
	xSemaphoreTake(i2c_semaphore,0);
	I2C_TransferReturn_TypeDef I2C_Status;
	I2C_TransferSeq_TypeDef seq;
	uint8_t data[2];

	seq.addr = device_addr;
	seq.flags = I2C_FLAG_WRITE_READ;

	seq.buf[0].data = &reg;
	seq.buf[0].len = 1;
	seq.buf[1].data = data;
	seq.buf[1].len = 1;

	I2C_Status = I2C_TransferInit(I2C1, &seq);

	while (I2C_Status == i2cTransferInProgress) {
		I2C_Status = I2C_Transfer(I2C1);
	}
	xSemaphoreGive(i2c_semaphore);
	if (I2C_Status != i2cTransferDone) {
		return false;
	}

	*val = data[0];

	return true;

}

bool my_i2c_test() {
	uint8_t data;

	my_i2c_read(0xD0, &data);

	printf("I2C: %02X\n", data);

	if (data == 0x60) {
		return true;
	} else {
		return false;
	}

}
