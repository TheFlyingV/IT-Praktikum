#include "PWM.h"

PWM::PWM() {}

PWM::~PWM() {
    *DIS = 1;
    *CPRD0 = 0;
    *CDTY0 = 0;
    SET_BIT(*GPIO, pin);
}

bool PWM::init(Configuration::s_PWMConfig* thisPWMConfig_) {
    pin = thisPWMConfig_->GPIO_pin;
    // init register
    VINTP GPER = (VINTP)(GPIO_MODULE + (thisPWMConfig_->GPIO_port ? GPIO_PORT_OFFSET : 0) + GPIO_GPER_OFFSET);
    VINTP PMR0 = (VINTP)(GPIO_MODULE + (thisPWMConfig_->GPIO_port ? GPIO_PORT_OFFSET : 0) + GPIO_PMR0_OFFSET);
    VINTP PMR1 = (VINTP)(GPIO_MODULE + (thisPWMConfig_->GPIO_port ? GPIO_PORT_OFFSET : 0) + GPIO_PMR1_OFFSET);
    // disable GPIO
    CLEAR_BIT(*GPER, pin);
    // set peripheral function
    if (BIT_IS_SET(thisPWMConfig_->GPIO_multiplexRegisterValue, 0)) SET_BIT(*PMR0, pin);
    else CLEAR_BIT(*PMR0, pin);
    if (BIT_IS_SET(thisPWMConfig_->GPIO_multiplexRegisterValue, 1)) SET_BIT(*PMR1, pin);
    else CLEAR_BIT(*PMR1, pin);
    channelID = thisPWMConfig_->channelID;
    // init register
    ENA = (VINTP)(PWM_MODULE + PWM_ENA_OFFSET);
    DIS = (VINTP)(PWM_MODULE + PWM_DIS_OFFSET);
    SR = (VINTP)(PWM_MODULE + PWM_SR_OFFSET);
    CMR0 = (VINTP)(PWM_MODULE + PWM_CMR0_OFFSET + PWM_CHANNEL_OFFSET * channelID);
    CUPD0 = (VINTP)(PWM_MODULE + PWM_CMR0_OFFSET + PWM_CHANNEL_OFFSET * channelID + PWM_CUPD0_OFFSET);
    CPRD0 = (VINTP)(PWM_MODULE + PWM_CMR0_OFFSET + PWM_CHANNEL_OFFSET * channelID + PWM_CPRD0_OFFSET);
    // set period value depending on frequency
    *CPRD0 = (int)(Configuration::PWMCLK / thisPWMConfig_->frequency);
    // toggle polarity
    SET_BIT(*CMR0, 9);
    CDTY0 = (VINTP)(PWM_MODULE + PWM_CMR0_OFFSET + PWM_CHANNEL_OFFSET * channelID + PWM_CDTY0_OFFSET);
    *CDTY0 = 0;
    // set maxPWMRatio
    maxPWMRatio = thisPWMConfig_->maxPWMRatio;
    return true;
}

bool PWM::setChannelPWMRatio(unsigned char ratioOn, bool capRatioOn) {
    if (capRatioOn) {
        // truncate ratioOn value
        if (ratioOn > maxPWMRatio) ratioOn = maxPWMRatio;
    } else
        // return false otherwise
        if (0 < ratioOn || ratioOn > maxPWMRatio) return false;
    printf("set ration: %d\n", (unsigned int) ratioOn);
    // set new duty cycle value
    *CUPD0 = (int)((float) ratioOn * (*CPRD0) / 255);
    // reset mode register pin 10 to initiate duty cycle update
    // @simulation
    CLEAR_BIT(*CMR0, 10);
    return true;
}

unsigned char PWM::getChannelPWMRatio() {
    // return current ratio
    return (char)(*CDTY0 * 255 / *CPRD0);
}

bool PWM::isChannelEnabled() {
    // check if bit is set
    return BIT_IS_SET(*SR, channelID);
}

bool PWM::setChannelEnabled(bool enabled) {
    // set ena/dis bit
    if (enabled) SET_BIT(*ENA, channelID);
    else SET_BIT(*DIS, channelID);
    return true;
}