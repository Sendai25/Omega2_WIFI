#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

using namespace std;

struct wifiData{
	string channel;
	string ssid;
	string encryption; //"NONE" for open network
	int signalStrength;
	wifiData* node;
};

bool jsonToCsvGeneral (const char* filename) {
    ofstream csv;
    ifstream json;
    char c;
    int counter = 0;
    int lim = 5;
    int close = 0;
    bool read = 0;
    int theEnd = 0;
    json.open(filename);
    csv.open("wifi_info.csv");
    while (!json.eof()) {
        json >> c;
        if (c == '"') {
            counter++;
            if (close%8 != 0 && close != 0)
                lim = 4;
        }
        if (read == 1 && c =='"') {
            read = 0;
            if (close%8 != 7)
                csv << ',';
            close++;
        }
        if (read == 1)
            csv << c;
        if (counter == lim) {
            read = 1;
            counter = 0;
        }
        if (c == '}') {
            if (close != theEnd) {
                csv << '\n';
                theEnd = close;
            }
        }
    }
    json.close();
    csv.close();
    if (close == 0)
        return 0;
    else
        return 1;
}

bool jsonToCsvGPS (const char* filename) {
    ofstream csv;
    ifstream json;
    char c;
    json.open(filename);
    csv.open("gps_info.csv");
    bool tellFalse = 0;
    bool read = 0;
    int close = 0;
    int counter = 0;
    int lim = 5;
    while (!json.eof()) {
        json >> c;
        if (tellFalse == 1 && c == 'l') {
            csv << "False";
            return 0;
        }
        if (c == 'a')
            tellFalse = 1;
        if (read == 1 && c == '"') {
            read = 0;
            close++;
            csv << ',';
        }
        if (read == 1) {
            csv << c;
        }
        if (counter == lim) {
            read = 1;
            counter = 0;
        }
        if (c == '"') {
            counter++;
            if (close > 0)
                lim = 4;
            if (close == 3)
                break;
        }
    }
    json.close();
    csv.close();
    return 1;
}

wifiData* addInfo(wifiData* head, string channel, string ssid, string encry, int strength) {
    wifiData* curr = new wifiData;
    curr->channel = channel;
    curr->ssid = ssid;
    curr->encryption = encry;
    curr->signalStrength = strength;
    curr->node = head;
    return curr;
}

void selection(int arr[], int size) {
	int pos_min;
	int temp;

	for (int i=0; i<size-1; i++){
		pos_min = i;
		for(int j=i+1; j<size; j++){
			if(arr[j] < arr[pos_min]){
				pos_min = j;
			}
		}
		if (pos_min != i){
			temp = arr[i];
			arr[i] = arr[pos_min];
			arr[pos_min] = temp;
		}
	}
}

int writeData (bool gpsStatus, bool generalStatus) {
    string channel;
    string ssid;
    string bssid;
    string auth;
    string encry;
    string strStrth;
    string wlsm;
    string ext;
    int strength;
    ofstream text;
    ifstream GPScsv;
    ifstream Generalcsv;
    text.open("Data.txt");
    if (generalStatus == 0) {
        text << "No wifi signal detected";
        text.close();
        return -1;
    }
    if (gpsStatus == 0) {
        text << "Warning: Cannot retrieve gps information.\n\n";
        wifiData* ini = NULL;
        Generalcsv.open("wifi_info.csv");
        while(Generalcsv.good()) {
            getline(Generalcsv,channel,',');
            getline(Generalcsv,ssid,',');
            getline(Generalcsv,bssid,',');
            getline(Generalcsv,auth,',');
            getline(Generalcsv,encry,',');
            getline(Generalcsv,strStrth,',');
            getline(Generalcsv,wlsm,',');
            getline(Generalcsv,ext,'\n');
            strength = atoi(strStrth.c_str());
            ini = addInfo(ini,channel,ssid,encry,strength);
        }
        wifiData* cursor = ini->node;
        int counter = 0;
        int strengthArrray[20];
        while (cursor != NULL) {
/*            text << "Wifi #" << counter << ":\n";
            text << "Name: " << cursor->ssid << '\n';
            text << "Channel: " << cursor->channel << '\n';
            text << "Encryption: " << cursor->encryption << '\n';
            text << "SignalStrength: " << cursor->signalStrength << "\n\n";
*/
            strengthArrray[counter] = cursor->signalStrength;
            cursor = cursor->node;
            counter++;
        }
        bool repeat = 0;
        selection(strengthArrray, counter);
        wifiData* sorted = NULL;
        wifiData* savedsorted = sorted;
        wifiData* sortercursor = ini->node;
        for (int i = 0; i < counter; i++) {
            while (sortercursor != NULL) {
                if (sortercursor->signalStrength == strengthArrray[i]) {
                    savedsorted = sorted;
                    while (sorted != NULL) {
                        if (sortercursor->ssid == sorted->ssid)
                            repeat = 1;
                        sorted = sorted->node;
                    }
                    sorted = savedsorted;
                    if (repeat == 0)
                        sorted = addInfo(sorted, sortercursor->channel, sortercursor->ssid, sortercursor->encryption, sortercursor->signalStrength);
                }
                sortercursor = sortercursor->node;
            }
            sortercursor = ini->node;
        }
        int counter2 = 1;
        while (sorted != NULL) {
            text << "Wifi #" << counter2;
            if (sorted->encryption == "NONE")
                text << " (Public):\n";
            else
                text << ":\n";
            text << "Name: " << sorted->ssid << '\n';
            text << "Channel: " << sorted->channel << '\n';
            text << "Encryption: " << sorted->encryption << '\n';
            text << "SignalStrength: " << sorted->signalStrength << "\n\n";
            sorted = sorted->node;
            counter2++;
        }
        return 1;
        text.close();
        GPScsv.close();
        Generalcsv.close();
    }
    if (gpsStatus == 1) {
        GPScsv.open("gps_info.csv");
        string latitude;
        string longitude;
        string elevation;
        wifiData* ini = NULL;
        while(GPScsv.good()) {
            getline(GPScsv,latitude,',');
            getline(GPScsv,longitude,',');
            getline(GPScsv,elevation);
        }
        Generalcsv.open("wifi_info.csv");
        while(Generalcsv.good()) {
            getline(Generalcsv,channel,',');
            getline(Generalcsv,ssid,',');
            getline(Generalcsv,bssid,',');
            getline(Generalcsv,auth,',');
            getline(Generalcsv,encry,',');
            getline(Generalcsv,strStrth,',');
            getline(Generalcsv,wlsm,',');
            getline(Generalcsv,ext,'\n');
            strength = atoi(strStrth.c_str());
            ini = addInfo(ini,channel,ssid,encry,strength);
        }
        wifiData* cursor = ini->node;
        int counter = 0;
        int strengthArrray[20];
        while (cursor != NULL) {
            strengthArrray[counter] = cursor->signalStrength;
            cursor = cursor->node;
            counter++;
        }
        bool repeat = 0;
        selection(strengthArrray, counter);
        wifiData* sorted = NULL;
        wifiData* savedsorted = sorted;
        wifiData* sortercursor = ini->node;
        for (int i = 0; i < counter; i++) {
            while (sortercursor != NULL) {
                if (sortercursor->signalStrength == strengthArrray[i]) {
                    savedsorted = sorted;
                    while (sorted != NULL) {
                        if (sortercursor->ssid == sorted->ssid)
                            repeat = 1;
                        sorted = sorted->node;
                    }
                    sorted = savedsorted;
                    if (repeat == 0)
                        sorted = addInfo(sorted, sortercursor->channel, sortercursor->ssid, sortercursor->encryption, sortercursor->signalStrength);
                }
                sortercursor = sortercursor->node;
            }
            sortercursor = ini->node;
        }
        int counter2 = 1;
        while (sorted != NULL) {
            text << "Wifi #" << counter2;
            if (sorted->encryption == "NONE")
                text << " (Public):\n";
            else
                text << ":\n";
            text << "Name: " << sorted->ssid << '\n';
            text << "Channel: " << sorted->channel << '\n';
            text << "Encryption: " << sorted->encryption << '\n';
            text << "SignalStrength: " << sorted->signalStrength << "\n";
            if (sorted->signalStrength >= 50) {
                text << "BetterSignalLocation: \n\tlatitude: " << latitude;
                text << "\n\tlongitude: " << longitude;
                text << "\n\televation: " << elevation << "\n\n";
            }
            else
                text << "BetterSignalLocation: N/A \n\n";
            sorted = sorted->node;
            counter2++;
        }
        return 0;
        text.close();
        GPScsv.close();
        Generalcsv.close();
    }

}

int main () {
//	system("ubus call onion wifi-scan \"{'device':'ra0'}\" > wifi_info.json");
//	system("ubus call gps info > gps_info.json");
    int General = jsonToCsvGeneral("wifi_info.json");
    int GPS = jsonToCsvGPS("gps_info.json");
    writeData(GPS, General);
    return 0;
}
