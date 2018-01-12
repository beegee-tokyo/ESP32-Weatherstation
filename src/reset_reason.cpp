#include "setup.h"
#include <rom/rtc.h>

String reset_reason(RESET_REASON reason)
{
	switch ( reason)
	{
		case 1	: return "Vbat power on reset";break;
		case 3	: return "Software reset digital core";break;
		case 4	: return "Legacy watch dog reset digital core";break;
		case 5	: return "Deep Sleep reset digital core";break;
		case 6	: return "Reset by SLC module, reset digital core";break;
		case 7	: return "Timer Group0 Watch dog reset digital core";break;
		case 8	: return "Timer Group1 Watch dog reset digital core";break;
		case 9	: return "RTC Watch dog Reset digital core";break;
		case 10 : return "Instrusion tested to reset CPU";break;
		case 11 : return "Time Group reset CPU";break;
		case 12 : return "Software reset CPU";break;
		case 13 : return "RTC Watch dog Reset CPU";break;
		case 14 : return "for APP CPU, reseted by PRO CPU";break;
		case 15 : return "Reset when the vdd voltage is not stable";break;
		case 16 : return "RTC Watch dog reset digital core and rtc module";break;
		default : return "NO_MEAN";
	}
}
