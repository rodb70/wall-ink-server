#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include "math.h"
#include "fonts.h"
#include "image.h"
#include "qr_code_generator/QrCode.hpp"
#include "processImage.h"
#define DEBUG 0

uint8_t* image;
uint32_t sleepTime;
GFXcanvas1* canvas;
uint16_t x_res;
uint16_t y_res;


void setPixel(uint32_t x, uint32_t y, unsigned char color) {
    if (x/8 + x_res/8*y >= y_res * x_res/8) {
        return;
    }
    if (color == 0) {
        image[x/8 + x_res/8*y] = image[x/8 + x_res/8*y] & ((0x01 << (7-x%8)) ^ 0xff); //white
    }
    else if (color == 1) {
        image[x/8 + x_res/8*y] = image[x/8 + x_res/8*y] | (0x01 << (7-x%8)); //black
    }
    else if (color == 2) {
        image[x/8 + x_res/8*y] = image[x/8 + x_res/8*y] ^ (0x01 << (7-x%8)); //swap
    }
    else if (color == 3 && (x+y) % 2 == 1) {
        image[x/8 + x_res/8*y] = image[x/8 + x_res/8*y] | (0x01 << (7-x%8)); //gray
    }
    else if (color == 3 && (x+y) % 2 == 0) {
        image[x/8 + x_res/8*y] = image[x/8 + x_res/8*y] & ((0x01 << (7-x%8)) ^ 0xff); //
    }
}

void drawRect(int x, int y, int width, int height, unsigned char color) {
    for (int x2 = 0; x2 < width; x2++) {
        for (int y2 = 0; y2 < height; y2++) {
            setPixel(x + x2, y + y2, color);
        }
    }
}

void initializeImage() {
    image = (uint8_t*) malloc((x_res * y_res)/8);
    for (int i = 0; i < x_res * y_res/8; i++) {
        image[i] = 0;
    }
}

void drawFancyString(std::string str, int16_t x, int16_t y) {
    canvas->setCursor(x, y);
    for (int i = 0; i < str.length(); i++)
        canvas->write(str[i]);
}

uint16_t getTextWidth(std::string str) {
    int16_t x1, y1;
    uint16_t w, h;
    canvas->getTextBounds(str.c_str(), 0, 50, &x1, &y1, &w, &h); //50 was arbitrarily chosen
    return w;
}

bool drawCenteredString(std::string str, int16_t y){
    uint16_t w = getTextWidth(str);
    if (w <= x_res)
    	drawFancyString(str, (x_res-w)/2, y);
    return w <= x_res;
}

uint16_t getQrCodeSize(std::string str) {
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(str.data(), qrcodegen::QrCode::Ecc::MEDIUM);
    return qr.getSize();
}

uint16_t putQrCode(uint16_t x, uint16_t y, std::string str, uint16_t scale) {
    if (str.length() != 0) {
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(str.data(), qrcodegen::QrCode::Ecc::MEDIUM);
        for (uint16_t y_offset = 0; y_offset < qr.getSize(); y_offset++) {
            for (uint16_t x_offset = 0; x_offset < qr.getSize(); x_offset++) {
                drawRect(x+x_offset*scale,y+y_offset*scale,scale,scale,qr.getModule(x_offset, y_offset));
            }
        }
        return qr.getSize();
    } else {
        return 0;
    }
}

unsigned char reverseByte(uint8_t x) {
    static const uint8_t table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };
    return table[x];
}

void invert(){
    for (int i = 0; i < x_res/8 * y_res; i++) {
        image[i] = image[i] ^ 0xff;
    }
}

void mirror() {
    char temp;
    for (int y = 0; y < y_res; y++) {
        for (int x = 0; x < x_res/16; x++) {
            temp = image[y * x_res/8 + x]; 
            image[y * x_res/8 + x] = image[y * x_res/8 + x_res/8 - 1 - x];
            image[y * x_res/8 + x_res/8 - 1 - x] = temp;
        }
    }
    for (int i = 0; i < y_res * x_res/8; i++) {
        //image[i] = reverseByte(image[i]);
        temp = 0;
        for(int j=0;j<8;j++)
            temp |= ((image[i]>>j) & 1)<<(7-j);
        image[i] = temp;
    }
}

void flip() {
    char temp;
    for (int y = 0; y < y_res/2; y++) {
        for (int x = 0; x < x_res/8; x++) {
            temp = image[y * x_res/8 + x]; 
            image[y * x_res/8 + x] = image[x_res/8*(y_res-1) - y*x_res/8 + x];
            image[x_res/8*(y_res-1) - y*x_res/8 + x] = temp;
        }
    }
}

std::string reservationBlockToTime(int block) {
    std::stringstream hourStream;
    hourStream << block/2 + 6;
    std::string hour = hourStream.str();
    if (hour.length() < 2) {
        hour = "0" + hour;
    }
    std::stringstream minuteStream;
    minuteStream << (block % 2) * 30;
    std::string minute = minuteStream.str();
    if (minute.length() < 2) {
        minute += "0";
    }
    return hour + ":" + minute;
}

std::string militaryTimeToNormalPersonTime(std::string military) {
    int hour = atoi(military.substr(0,2).c_str());
    std::string ampm = "am";
    if (hour > 11)
        ampm = "pm";
    if (hour > 12)
        hour -= 12;
    if (hour == 0)
        hour = 12;
    std::stringstream hourStream;
    hourStream << hour;
    return hourStream.str() + ":" + military.substr(3,2) + ampm;
}

std::string fancyDateFromYYYY_MM_DD(std::string YYYY_MM_DD) {
    int year = atoi(YYYY_MM_DD.substr(0,4).c_str());
    int month = atoi(YYYY_MM_DD.substr(5,2).c_str());
    int day = atoi(YYYY_MM_DD.substr(8,2).c_str());
    std::string months[12] = {"January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"};
    std::string monthName = months[month-1];
    std::stringstream fancyDate;
    fancyDate << monthName << " " << day << ", " << year;
    return fancyDate.str();
}

std::vector<reservation> parseReservations(std::string* reservations) {
    std::vector<reservation> reservs;
    reservs.clear();
    int startBlock = 0;
    std::string title = reservations[0];
    for (int block = 1; block < 32; block++) {
        if (title.compare(reservations[block]) != 0) {
            reservation r;
            r.title = title;
            r.startBlock = startBlock;
            r.endBlock = block;
            reservs.push_back(r);
            title = reservations[block];
            startBlock = block;
        }
    }
    reservation r;
    r.title = title;
    r.startBlock = startBlock;
    r.endBlock = 32;
    reservs.push_back(r);
    return reservs;
}

void checkBattery(int xOffset, int yOffset, float voltage) {
    if (voltage <= 2.5) {
        //draw low battery symbol
        drawRect(xOffset, yOffset, 48, 23, 1);
        drawRect(xOffset+2, yOffset+2, 44, 19, 0);
        drawRect(xOffset+48, yOffset+5, 4, 13, 1);
        drawRect(xOffset+48, yOffset+7, 2, 9, 0);
        for (int i = 0; i < 36; i++) {
            drawRect(xOffset+41-i, yOffset-7+i, 3, 1, 1);
        }
    }
}

uint32_t setSleepTime(uint32_t increment) { //increment is the target number of seconds between refreshes
    time_t currentTimeTemp = time(nullptr);
    uint32_t currentTime = currentTimeTemp;
    if (((currentTime+TIME_ZONE*60*60) % 86400) > 77400) { //if time is past 9:30pm, wake at 6:30am
        sleepTime = 86400 + 23400 - ((currentTime+TIME_ZONE*60*60) % 86400);
    } else if (((currentTime+TIME_ZONE*60*60) % 86400) < 23000) { //if time is before 6:30am, wake at 6:30am
        sleepTime = 23400 - ((currentTime+TIME_ZONE*60*60) % 86400);
    } else {
        sleepTime = increment - (currentTime % increment);
    }
    //wake 2 minutes early so that it is showing the schedule for the next block ahead of time
    if (sleepTime > 180 ) {
        sleepTime -= 120;
    } else if (increment > 120) {
        sleepTime += increment;
        sleepTime -= 120;
    }

#if DEBUG == 2
    cout << "Current Time: " << currentTime % 86400 << endl;
    cout << "Current Time with 7 hour offset: " << (currentTime-7*60*60) % 86400 << endl;
    cout << "Sleep Time: " << sleepTime << endl;
#endif
    return sleepTime;
}

//found at https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
std::string replaceString(std::string subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

std::string fixPunctuation(std::string input) {
    std::string output = replaceString(input, "&quot;", "\"");
    output = replaceString(output, "&amp;", "&");
    output = replaceString(output, "&lt;", "<");
    output = replaceString(output, "&gt;", ">");
    return output;
}

int main(int argc, char* argv[]) {
    //read from the database info
    std::ifstream fromDB;
    if (argc == 1)
        fromDB.open("fromDB");
    else
        fromDB.open(argv[1]);
    std::string mac_address;
    getline(fromDB, mac_address);
    std::string dateNow;
    getline(fromDB, dateNow);
    std::string timeNow;
    getline(fromDB, timeNow);
    std::string deviceType;
    getline(fromDB, deviceType);
    std::string voltage;
    getline(fromDB, voltage);
    std::string orientation;
    getline(fromDB, orientation);
    std::string resourceID;
    getline(fromDB, resourceID);
    std::string displayUrl;
    getline(fromDB, displayUrl);
    std::string qrCodeBaseUrlBeginning;
    getline(fromDB, qrCodeBaseUrlBeginning);
    std::string qrCodeBaseUrlEnd;
    getline(fromDB, qrCodeBaseUrlEnd);
    std::string name;
    getline(fromDB, name);

    if (deviceType.compare("0") == 0) {
        x_res = X_RES0;
        y_res = Y_RES0;
    } else if (deviceType.compare("1") == 0) {
        x_res = X_RES1;
        y_res = Y_RES1;
    } else if (deviceType.compare("2") == 0) {
        x_res = X_RES2;
        y_res = Y_RES2;
    } else if (deviceType.compare("3") == 0) {
        x_res = X_RES3;
        y_res = Y_RES3;
    } else if (deviceType.compare("4") == 0) {
        x_res = X_RES4;
        y_res = Y_RES4;
    } else if (deviceType.compare("6") == 0) {
        x_res = X_RES4;
        y_res = Y_RES4;
    } else if (deviceType.compare("7") == 0) {
        x_res = X_RES2;
        y_res = Y_RES2;
    } else if (deviceType.compare("9") == 0) {
        x_res = X_RES4;
        y_res = Y_RES4;
    } else if (deviceType.compare("10") == 0) {
        x_res = X_RES2;
        y_res = Y_RES2;
    } else if (deviceType.compare("11") == 0) {
        x_res = X_RES3;
        y_res = Y_RES3;
    }
    canvas = new GFXcanvas1(x_res, y_res);
    canvas->fillScreen(0);
    image = canvas->getBuffer();

    std::string reservations[32];
    for (int i = 0; i < 32; i++) {
        reservations[i] = "Available";
    }
    std::string title;
    int eventNum = 0;

    //Open the fromDB file and parse that info into the reservations array. Will probably need to be changed later to add more functionality.
    while (getline(fromDB, title)) {
        //fix problems like the & symbol being shown as &amp;
        title = fixPunctuation(title);

        //Make the titles not collide, ever
        ++eventNum;
        for (int i = 0; i < eventNum; i++) {
            title += "\r";
        }

        //Take in a date formatted string and decide which reservations[] time block it corresponds to
        std::string dateTimeStart;
        getline(fromDB, dateTimeStart);
        int startIndex;
        if (dateNow.compare(dateTimeStart.substr(0,10)) == 0) {
            int hour = atoi(dateTimeStart.substr(11,2).c_str());
            int minute = atoi(dateTimeStart.substr(14,2).c_str());
            hour -= 6;
            hour *= 2;
            minute /= 30;
            startIndex = hour + minute;
            if (startIndex < 0)
                startIndex = 0;
            if (startIndex > 30)
                startIndex = 30;
        } else {
            startIndex = 0;
        }

        //Take in a date formatted string and decide which reservations[] time block it corresponds to
        std::string dateTimeEnd;
        getline(fromDB, dateTimeEnd);
        int endIndex;
        if (dateNow.compare(dateTimeEnd.substr(0,10)) == 0) {
            int hour = atoi(dateTimeEnd.substr(11,2).c_str());
            int minute = atoi(dateTimeEnd.substr(14,2).c_str());
            hour -= 6;
            hour *= 2;
            minute /= 30;
            endIndex = hour + minute;
            if (endIndex < 0)
                endIndex = 0;
            if (endIndex > 32)
                endIndex = 32;
        } else {
            endIndex = 32;
        }

        for (int i = startIndex; i < endIndex; i++) {
            reservations[i] = title;
        }
    }

    //actually generate the desired image
    if (deviceType.compare("0") == 0) {
        drawImage0(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("1") == 0) {
        drawImage1(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("2") == 0) {
        drawImage2(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("3") == 0) {
        drawImage3(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("4") == 0) {
        drawImage4(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("6") == 0) {
        drawImage6(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("7") == 0) {
        drawImage7(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("9") == 0) {
        drawImage9(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("10") == 0) {
        drawImage10(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    } else if (deviceType.compare("11") == 0) {
        drawImage11(name, dateNow, timeNow, reservations, stof(voltage), resourceID, displayUrl, qrCodeBaseUrlBeginning, qrCodeBaseUrlEnd);
    }

    //if orientation is 1, flip image
    if (orientation.compare("1") == 0) {
        flip();
        mirror();
    }

    std::vector<unsigned char> processed = processImage(image, sleepTime, x_res, y_res, mac_address);
    //write to a file
    std::ofstream("image_data/" + mac_address, std::ios::binary).write((const char*) image, x_res/8 * y_res);
    std::ofstream("image_data/" + mac_address + ".wink", std::ios::binary).write((const char*) processed.data(), processed.size());

    //free memory
    delete canvas;
    return 0;
}
