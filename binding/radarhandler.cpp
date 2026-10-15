#include "radarhandler.h"
#ifdef CONFIG_SPI_ENABLE
#include "radarif_ll_spi.h"
#endif
#include "radarif_ll_uart.h"
#include "stdio.h"
#ifndef WIN
#include "unistd.h"
#else
#include <windows.h>
#include "synchapi.h"
#endif
#include "string.h"
#include <stddef.h>
#include <stdexcept>
// ahm_cmds
#include "ahm_comm_def.h"
#include "ahm_common.h"
// wiring pi source
#include "wiringSerial.h"
// radar data
#include "radarData.h"
// comunication
#include "radarif_ll.h"
// radar handler
#include "radar_AHM2D.h"
#include "radar_AHM3D.h"
#include "radar_AHM2DL.h"

#include "radar_api_base.h"

#define BAUD_RATE 921600

#ifdef WIN
#define usleep(x) Sleep(x/1000)
#endif

//std::string RadarHandler::COM = "";

// functions used for uart comunication

/**
* This routine inquiry the device ID and instance the handler
* */
radar_api_base * RadarHandler::openDevice(rifll_h * comm_h){
    uint16_t hwCode;
        if (radar_api_base::getHW(comm_h, &hwCode)){
            printf("Error on retrieve HWID\n");
            return NULL;
        }
        if (hwCode == rdr_AHM2D::HWID){
            printf("Detected AHM2D\n");
            return new radar_AHM2D(comm_h);
        }
        if (hwCode == rdr_AHM3D::HWID){
            printf("Detected AHM3D\n");
            return new radar_AHM3D(comm_h);
        }
        if (hwCode == rdr_AHM2DL::HWID){
            printf("Detected AHM2DL\n");
            return new radar_AHM2DL(comm_h);
        }
            printf("Unknown radar ID %x\n", hwCode);
            return NULL;
}


void RadarHandler::priv_identifyAndSetup(std::string serial_comunication , std::string comboard, int baudrate){
	//mp_interface parsing
	rifll_h* comm_h;
    printf("%s\n", comboard.c_str());
	bool interfaceValid = false;
    if (serial_comunication == "UART"){
        interfaceValid = true;
        comm_h = new radarif_ll_uart(comboard, baudrate);
        if (comm_h->isValid() == 0){
            throw std::runtime_error("unable to open serial port.");
        }
    }
#ifdef CONFIG_SPI_ENABLE
    if (serial_comunication == "SPI"){
        interfaceValid = true;
        comm_h = new radarif_ll_spi(&comboard[0], 0, 10000000UL, 0); // verificare bene cosa richiede e a cosa serve il primo carattere
    }
#endif
    if (!interfaceValid){
#ifdef CONFIG_SPI_ENABLE
        throw std::runtime_error("Invalid type of serial communication. Choose between 'UART' or 'SPI'.");
#else
		throw std::runtime_error("Invalid type of serial communication. Only UART supported.");
#endif
    }
    rdr_h = openDevice(comm_h);
    if (rdr_h == NULL)
        throw std::runtime_error("Device not found.");
}

// Constructor no BR
RadarHandler::RadarHandler(std::string serial_comunication , std::string comboard) 
{
    priv_identifyAndSetup(serial_comunication, comboard, BAUD_RATE);
}
// Constructor with BR
RadarHandler::RadarHandler(std::string serial_comunication , std::string comboard, int baudrate)
{
    priv_identifyAndSetup(serial_comunication, comboard, baudrate);
}

int RadarHandler::rdr_start() {
    return rdr_h->rdr_start();
};
int RadarHandler::rdr_stop() {
    return rdr_h->rdr_stop();
};
int RadarHandler::rdr_set_frame_rate(uint16_t p) {
    return rdr_h->rdr_set_frame_rate(p);
};
int RadarHandler::rdr_get_frame_rate(uint16_t * p) {
    return rdr_h->rdr_get_frame_rate(p);
};
int RadarHandler::rdr_set_tx_power(uint8_t p) {
    return rdr_h->rdr_set_tx_power(p);
};
int RadarHandler::rdr_get_tx_power(uint8_t * p) {
    return rdr_h->rdr_get_tx_power(p);
};
int RadarHandler::rdr_set_min_Xrange (float p) {
    return rdr_h->rdr_set_min_Xrange(p);
};
int RadarHandler::rdr_get_min_Xrange (float * p) {
    return rdr_h->rdr_get_min_Xrange(p);
};
int RadarHandler::rdr_set_max_Xrange (float p) {
    return rdr_h->rdr_set_max_Xrange(p);
};
int RadarHandler::rdr_get_max_Xrange (float * p) {
    return rdr_h->rdr_get_max_Xrange(p);
};
int RadarHandler::rdr_set_offset (float p) {
    return rdr_h->rdr_set_offset(p);
};
int RadarHandler::rdr_get_offset (float * p) {
    return rdr_h->rdr_get_offset(p);
};
int RadarHandler::rdr_set_iterations (uint16_t p) {
    return rdr_h->rdr_set_iterations(p);
};
int RadarHandler::rdr_get_iterations (uint16_t * p) {
    return rdr_h->rdr_get_iterations(p);
}
int RadarHandler::rdr_set_declutter_length (uint16_t p) {
    return rdr_h->rdr_set_declutter_length(p);
};
int RadarHandler::rdr_get_declutter_length (uint16_t * p) {
    return rdr_h->rdr_get_declutter_length(p);
};
int RadarHandler::rdr_set_elabtype(rdrElabTypes_t p) {
    return rdr_h->rdr_set_elabtype(p);
};
int RadarHandler::rdr_get_elabtype(rdrElabTypes_t * p) {
    return rdr_h->rdr_get_elabtype(p);
};
int RadarHandler::rdr_set_optproc(uint8_t opt0, uint8_t opt1) {
    return rdr_h->rdr_set_optproc(opt0, opt1);
};
int RadarHandler::rdr_get_optproc(uint8_t *opt0, uint8_t *opt1) {
    return rdr_h->rdr_get_optproc(opt0, opt1);
};
int RadarHandler::rdr_set_sequence(uint8_t * txSeqMask, uint8_t* rxSeqMask, uint8_t numSeq) {
    return rdr_h->rdr_set_sequence(txSeqMask, rxSeqMask, numSeq);
};
int RadarHandler::rdr_get_sequence(uint8_t * txSeqMask, uint8_t* rxSeqMask, uint8_t * numSeq) {
    return rdr_h->rdr_get_sequence(txSeqMask, rxSeqMask, numSeq);
};
int RadarHandler::rdr_set_freq_carrier(uint16_t carrierMHz) {
    return rdr_h->rdr_set_freq_carrier(carrierMHz);
};
int RadarHandler::rdr_get_freq_carrier(uint16_t * carrierMHz) {
    return rdr_h->rdr_get_freq_carrier(carrierMHz);
};
int RadarHandler::rdr_set_bw(uint16_t bwMHz) {
    return rdr_h->rdr_set_bw(bwMHz);
};
int RadarHandler::rdr_get_bw(uint16_t * bwMHz) {
    return rdr_h->rdr_get_bw(bwMHz);
};
int RadarHandler::rdr_set_bwmode(uint8_t bwmode) {
    return rdr_h->rdr_set_bwmode(bwmode);
};
int RadarHandler::rdr_get_bwmode(uint8_t * bwmode) {
    return rdr_h->rdr_get_bwmode(bwmode);
};
int RadarHandler::rdr_set_code(int8_t *code, int size) {
    return rdr_h->rdr_set_code(code, size);
};
int RadarHandler::rdr_get_code(int8_t *code, int *size) {
    return rdr_h->rdr_get_code(code, size);
};
int RadarHandler::rdr_set_gain(uint16_t gainI, uint16_t gainQ) {
    return rdr_h->rdr_set_gain(gainI, gainQ);
};
int RadarHandler::rdr_get_gain(uint16_t * gainI, uint16_t *gainQ) {
    return rdr_h->rdr_get_gain(gainI, gainQ);
};
int RadarHandler::rdr_get_fSampling(uint16_t * fMhz) {
    return rdr_h->rdr_get_fSampling(fMhz);
};
int RadarHandler::rdr_get_arrayDescr(txAntDesc_t * p1, rxAntDesc_t * p2) {
    return rdr_h->rdr_get_arrayDescr(p1, p2);
};
int RadarHandler::rdr_set_canvas(canvas2d_param_t p) {
    return rdr_h->rdr_set_canvas(p);
};
int RadarHandler::rdr_get_canvas(canvas2d_param_t * p) {
    return rdr_h->rdr_get_canvas(p);
};
int RadarHandler::rdr_set_fmt(uint8_t fmt) {
    return rdr_h->rdr_set_fmt(fmt);
};
int RadarHandler::rdr_get_fmt(uint8_t * fmt) {
    return rdr_h->rdr_get_fmt(fmt);
};
int RadarHandler::rdr_rx_multichannel(radarData<cplx_float_t> * prd) {
    return rdr_h->rdr_rx_multichannel(prd);
};
int RadarHandler::rdr_get_image(radarData<float> * prd) {
    return rdr_h->rdr_get_image(prd);
};
int RadarHandler::rdr_set_baudrate(uint32_t br) {
    return rdr_h->rdr_set_baudrate(br);
};
int RadarHandler::rdr_set_multistream(uint16_t nstream) {
    return rdr_h->rdr_set_multistream(nstream);
};
int RadarHandler::rdr_get_multistream(uint16_t *pnstream) {
    return rdr_h->rdr_get_multistream(pnstream);
};
int RadarHandler::rdr_conv_mask2sequence(uint8_t * txMask, uint8_t * rxMask, scanSequence_t * pseq, int num) {
    return rdr_h->rdr_conv_mask2sequence(txMask, rxMask, pseq, num);
};
int RadarHandler::rdr_set_canvas(canvas3d_param_t p) {
    return rdr_h->rdr_set_canvas(p);
};
int RadarHandler::rdr_get_canvas(canvas3d_param_t * p) {
    return rdr_h->rdr_get_canvas(p);
};
int RadarHandler::rdr_set_profile(profile_t p) {
    return rdr_h->rdr_set_profile(p);
};
int RadarHandler::rdr_set_staticObjRemoval(uint8_t enable) {
    return rdr_h->rdr_set_staticObjRemoval(enable);
};
int RadarHandler::rdr_get_staticObjRemoval(uint8_t * penable) {
    return rdr_h->rdr_get_staticObjRemoval(penable);
};
int RadarHandler::rdr_set_staticObjMapUpdateTime(float timeConstant) {
    return rdr_h->rdr_set_staticObjMapUpdateTime(timeConstant);
};
int RadarHandler::rdr_get_staticObjMapUpdateTime(float * ptimeConstant) {
    return rdr_h->rdr_get_staticObjMapUpdateTime(ptimeConstant);
};
int RadarHandler::rdr_set_rxGain(uint8_t val) {
    return rdr_h->rdr_set_rxGain(val);
};
int RadarHandler::rdr_get_rxGain(uint8_t * pval) {
    return rdr_h->rdr_get_rxGain(pval);
};
int RadarHandler::rdr_set_embeddedImgAlgo(uint8_t enable) {
    return rdr_h->rdr_set_embeddedImgAlgo(enable);
};
int RadarHandler::rdr_get_embeddedImgAlgo(uint8_t* penable) {
    return rdr_h->rdr_get_embeddedImgAlgo(penable);
};
RadarHandler::~RadarHandler(){};

