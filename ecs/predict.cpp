#include "predict.h"
#include "allocate.h"
#define SUM_UP 1
struct PhyInfo{
    int phycpu;
    int phymem;
    int phyhard;
} phyinfo;
struct FlavorsInfo{
    vector<string> vflavors;
    vector<int> vcpus, vmems;
} flavorsinfo;

int predict_daySpan, history_daySpan;
vector<int> vflavors_pridict_nums;
string dim, predict_begin_time, predict_end_time, history_begin_time, history_end_time;
vector< vector<int> > sequences;
time_t predict_begin_time_t, predict_end_time_t, history_begin_time_t, history_end_time_t;
int oneDayLong = 24 * 3600; //s

double alpha[16]={0.9,0.1,0.2,    0.8,0.7,0.9,    0.1,0.9,0.9,    0.1,0.9,0.9,    0.9,0.5,0.5,0.5};
double beta[16]={0.3,0.7,0.5,    0.7,0.5,0.6,    0.8,0.5,0.5,    0.5,0.7,0.4,    0.4,0.5,0.6,0.5,};
double gama[16]={0.3,0.1,0.9,    0.9,0.7,0.9,    0.9,0.5,0.2,    0.2,0.1,0.5,    0.1,0.2,0.8,0.5,};

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
    get_info(info);
    get_data(data, data_num);
    //average_predict();
    three_predict();
    allocate_simple(flavorsinfo.vflavors, vflavors_pridict_nums, dim, filename);
}

void three_predict(){
    int flag = 3;
    int  Period = predict_daySpan;
    //three predict
    for(int i=0; i<sequences.size(); i++){
        // s t p 
        int predict_value=0;
        double sequence_now;
        vector<double> predict_s;
        vector<double> predict_t;
        vector<double> predict_p;
        predict_s.push_back(sequences[i][0]);
        predict_t.push_back(sequences[i][1]-sequences[i][2]);
        predict_p.push_back(0);
        for(int j=1; j<sequences[i].size(); j++){
            double pii_k;
            double temp_s,temp_t,temp_p;
            if(j<=Period){
               pii_k = 0; 
            }
            else{
                pii_k= predict_p[j-Period];
            }
            sequence_now = (double)sequences[i][j]; 
            temp_s = alpha[i]*(sequence_now - pii_k) + (1-alpha[i])*(predict_s[j-1] + predict_t[j-1]);
            temp_t = beta[i]*(temp_s-predict_s[j-1])+(1-beta[i])*predict_t[j-1];
            temp_p = gama[i]*(sequence_now-temp_s)+(1-gama[i])*pii_k;
            predict_s.push_back(temp_s);
            predict_t.push_back(temp_t);
            predict_p.push_back(temp_p);
            if(flag==i){
                ;
            //    cout <<j<< ": "<< temp_s<<" "<<temp_t<<" "<<temp_p<<endl;
            }
        }
        //try to predict
            predict_value=(predict_s[predict_s.size()-1]+predict_daySpan*predict_t[history_daySpan-1])+predict_p[history_daySpan+Period+predict_daySpan%Period-1];
            predict_value = predict_value - sequences[i][sequences[i].size()-1];
            if(predict_value<0)
                predict_value = 0;
            cout << predict_value<<endl; 
            vflavors_pridict_nums.push_back(predict_value);
    }
}

void average_predict(){
    ;
}

void get_info(char * info[MAX_INFO_NUM])
{
    int num_of_vm;

    string line = info[0];
    int firstSpace = line.find_first_of(' ');
    int secondSpace = line.find_last_of(' ');
    phyinfo.phycpu = atoi(line.substr(0,firstSpace).c_str());
    phyinfo.phymem = atoi(line.substr(firstSpace+1, secondSpace-firstSpace-1).c_str())*1024; //MB
    phyinfo.phyhard = atoi(line.substr(secondSpace+1).c_str());

    line = info[2];
    num_of_vm = atoi(line.c_str());

    for(int i=3; i<3+num_of_vm; i++)
    {
        line = info[i];
        int firstSpace = line.find_first_of(' ');
        int secondSpace = line.find_last_of(' ');
        flavorsinfo.vflavors.push_back(line.substr(0,firstSpace));
        flavorsinfo.vcpus.push_back(atoi(line.substr(firstSpace+1, secondSpace-firstSpace-1).c_str()));
        flavorsinfo.vmems.push_back(atoi(line.substr(secondSpace+1).c_str()));
    }   

    dim = info[3+num_of_vm+1];
    predict_begin_time = info[3+num_of_vm+3];
    predict_end_time = info[3+num_of_vm+4];
    predict_begin_time_t = str2time(predict_begin_time.c_str(), "%Y-%m-%d %H:%M:%S");
    predict_end_time_t = str2time(predict_end_time.c_str(), "%Y-%m-%d %H:%M:%S");
    predict_daySpan = ceil((float)(predict_end_time_t - predict_begin_time_t)/oneDayLong);

    //对flavors排序，flavor1~flavor15  加了这一步骤，对结果没影响
    for(int i=0; i<flavorsinfo.vflavors.size()-1; i++)
    {
        for(int j=i+1; j<flavorsinfo.vflavors.size(); j++)
        {
            if(atoi(flavorsinfo.vflavors[j].substr(sizeof("flavor")-1).c_str()) < atoi(flavorsinfo.vflavors[i].substr(sizeof("flavor")-1).c_str()))
            {
                //交换
                string vflavor_tmp;
                int vcpu_tmp, vmem_temp;

                vflavor_tmp = flavorsinfo.vflavors[i];
                vcpu_tmp    = flavorsinfo.vcpus[i];
                vmem_temp   = flavorsinfo.vmems[i];

                flavorsinfo.vflavors[i] = flavorsinfo.vflavors[j];
                flavorsinfo.vcpus[i]    = flavorsinfo.vcpus[j];
                flavorsinfo.vmems[i]    = flavorsinfo.vmems[j];

                flavorsinfo.vflavors[j] = vflavor_tmp;
                flavorsinfo.vcpus[j]    = vcpu_tmp;
                flavorsinfo.vmems[j]    = vmem_temp;
            }
        }
    }

    cout << "phycpu: " << phyinfo.phycpu << endl;
    cout << "phymem: " << phyinfo.phymem << endl;
    cout << "phyhard: " << phyinfo.phyhard << endl;
    cout << "num_of_vm: " << num_of_vm << endl;
    for(int i=0; i<flavorsinfo.vflavors.size(); i++)
    {
        cout << flavorsinfo.vflavors[i]<<" "<< flavorsinfo.vcpus[i] << " " << flavorsinfo.vmems[i]<<endl;
    }
    cout << "dim: " << dim <<endl;
    cout << "predict_begin_time: " << predict_begin_time << endl;
    cout << "predict_end_time: " << predict_end_time << endl;
    cout << "predict_daySpan: " << predict_daySpan << endl;
}
void get_data(char * data[MAX_DATA_NUM], int data_num)
{
    string line = data[0];
    int firstTab = line.find_first_of('\t');
    int secondTab = line.find_last_of('\t');
    int lastSpace = line.find_last_of(' ');
    history_begin_time = line.substr(secondTab+sizeof("\t")-1, lastSpace-secondTab-1);
    history_begin_time.append(" 00:00:00");
    cout << history_begin_time <<endl;
    line = data[data_num-1];
    firstTab = line.find_first_of('\t');
    secondTab = line.find_last_of('\t');
    lastSpace = line.find_last_of(' ');
    history_end_time = line.substr(secondTab+sizeof("\t")-1, lastSpace-secondTab-1);
    history_end_time.append(" 23:59:59");
    cout << history_end_time <<endl;

    history_begin_time_t = str2time(history_begin_time.c_str(), "%Y-%m-%d %H:%M:%S");
    cout << history_begin_time_t << endl;
    history_end_time_t = str2time(history_end_time.c_str(), "%Y-%m-%d %H:%M:%S");
    cout << history_end_time_t << endl;
    history_daySpan = ceil((float)(history_end_time_t - history_begin_time_t)/oneDayLong);
    cout << "history_daySpan: " << history_daySpan << endl;

    // get sequences
    for(int i=0; i<flavorsinfo.vflavors.size(); i++)
    {
        vector<int> sequence(history_daySpan, 0); //history_daySpan ints with value 0;
        //search data
        for(int j=0; j<data_num; j++)
        {
            string line = data[j];
            int firstTab = line.find_first_of('\t');
            int secondTab = line.find_last_of('\t');
            int lastSpace = line.find_last_of(' ');
            string vflavor = line.substr(firstTab+sizeof("\t")-1, secondTab-firstTab-sizeof("\t")+1);
            time_t time =  str2time(line.substr(secondTab+sizeof("\t")-1, lastSpace-secondTab-1).append(" 00:00:00").c_str(), "%Y-%m-%d %H:%M:%S");
            int dayDiff = (time - history_begin_time_t)/oneDayLong; // used as sequence index
//            cout << vflavor << "\t" << time << "\t" << dayDiff << endl;
            if(vflavor == flavorsinfo.vflavors[i]) //this kind of flavor need to be predicted
            {
                sequence[dayDiff]++;   //add a record at daydiff index
            }
        }
        sequences.push_back(sequence);
    }

    for(int i=0; i<sequences.size(); i++)
    {
       // cout << flavorsinfo.vflavors[i] << endl;
        int sum = 0;
        for(int j=0; j<sequences[i].size(); j++)
        {   sum = sequences[i][j] + sum; 
            if(SUM_UP){
                sequences[i][j]=sum;
            }
          //  cout << sequences[i][j] << ", ";
        }
        //cout << "total_num: " << sum << endl;
        //cout << endl;
    }
}

time_t str2time(string str, string format)
{
    struct tm *tmp_time = (struct tm*)malloc(sizeof(struct tm));
    strptime(str.c_str(), format.c_str(), tmp_time);
    time_t t = mktime(tmp_time);
    free(tmp_time);
    return t;
}
