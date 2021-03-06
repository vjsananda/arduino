/**
 * @file       WidgetSD.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef WidgetSD_h
#define WidgetSD_h

#include <Blynk/BlynkApi.h>
#include <SD.h>

class WidgetSD
{
public:
    WidgetSD() {}
    void onWrite(BlynkReq& request, const BlynkParam& param);

private:
};

void WidgetSD::onWrite(BlynkReq& request, const BlynkParam& param)
{
    const char* cmd = param[0].asStr();
    if (!strcmp(cmd, "ls")) {
        if (File dir = SD.open(param[1].asStr())) {
            while (File entry = dir.openNextFile()) {
                char mem[32] = "";
                BlynkParam result(mem, 0, sizeof(mem));
                result.add(entry.name());
                if (entry.isDirectory()) {
                    result.add("/");
                } else {
                    result.add(entry.size());
                }
                Blynk.virtualWrite(request.pin, result);
                entry.close();
            }
            dir.close();
        }
    }  else if (!strcmp(cmd, "get")) {	// dc dc dc dc d[l|e]
        if (File f = SD.open(param[1].asStr())) {
            if (!f.isDirectory()) {
                char mem[32] = "dc";
                const int maxlen = sizeof(mem)-3;
                int len;
                do {
                    len = f.read(mem+3, maxlen);
                    if (len < 0) {
                        mem[1] = 'e'; // Error!
                        len = 0;
                    } else if (len < maxlen) {
                        mem[1] = 'l'; // Last chunk
                    }
                    Blynk.virtualWrite(request.pin, mem, len + 3);
                } while (len == maxlen);
            } else {

            }

            f.close();
        }
    } else {
        BLYNK_LOG("Invalid SD command: %s", cmd);
    }
}

#endif
