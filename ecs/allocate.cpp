#include "allocate.h"
extern struct PhyInfo{
    int phycpu;
    int phymem;
    int phyhard;
} phyinfo;

extern struct FlavorsInfo{
    vector<string> vflavors;
    vector<int> vcpus, vmems;
} flavorsinfo;

struct deploy{
    int phyid;
    vector<string> vflavors;
    vector<int> nums;  //能放置该类型虚拟机个数
    int cpuLeft;
    int memLeft;
};

void allocate_simple(vector<string> vflavors, vector<int> vflavors_pridict_nums, string dim, char* filename)
{
    vector<int> vflavors_pridict_nums_copy = vflavors_pridict_nums;
    vector<struct deploy> deploys;

    //open new physical machine
    struct deploy new_deploy;
    new_deploy.phyid = deploys.size() + 1;
    new_deploy.vflavors = flavorsinfo.vflavors;
    vector<int> nums(flavorsinfo.vflavors.size(), 0);
    new_deploy.nums = nums;
    new_deploy.cpuLeft = phyinfo.phycpu;
    new_deploy.memLeft = phyinfo.phymem;
    deploys.push_back(new_deploy);

    for(int i=vflavors.size()-1; i>=0; i--)
    {
        struct deploy new_deploy = deploys.back();
        while(vflavors_pridict_nums[i]) //need to be deployed
        {
            if(flavorsinfo.vcpus[i] <= new_deploy.cpuLeft && flavorsinfo.vmems[i] <= new_deploy.memLeft) //has space
            {
                new_deploy.nums[i]++;  //deploy
                new_deploy.cpuLeft -= flavorsinfo.vcpus[i]; //resources substract
                new_deploy.memLeft -= flavorsinfo.vmems[i];
                vflavors_pridict_nums[i]--;  //vm substract;
            }
            else  //space not enough
            {
                int j=i-1;
                for(j; j>=0; j--) //seek for small vm
                {
                    if(flavorsinfo.vcpus[j] <= new_deploy.cpuLeft && flavorsinfo.vmems[j] <= new_deploy.memLeft) //has space
                    {
                        new_deploy.nums[j]++;  //deploy
                        new_deploy.cpuLeft -= flavorsinfo.vcpus[j]; //resources substract
                        new_deploy.memLeft -= flavorsinfo.vmems[j];
                        vflavors_pridict_nums[j]--;  //vm substract;
                        break;  //only got one and break
                    }
                }

                if(j>=0) //got a small one
                {
                    continue;
                }
                else    //the cpuLeft or memLeft can not fit any vm
                {
                    deploys.erase(deploys.end());
                    deploys.push_back(new_deploy);  //complete this deploy
                    //and open new physical machine
                    struct deploy new_deploy;
                    new_deploy.phyid = deploys.size() + 1;
                    new_deploy.vflavors = flavorsinfo.vflavors;
                    vector<int> nums(flavorsinfo.vflavors.size(), 0);
                    new_deploy.nums = nums;
                    new_deploy.cpuLeft = phyinfo.phycpu;
                    new_deploy.memLeft = phyinfo.phymem;
                    deploys.push_back(new_deploy);
                    break;
                }

            }
        }
        if(vflavors_pridict_nums[i]==0 || deploys.back().cpuLeft != phyinfo.phycpu || deploys.back().memLeft != phyinfo.phymem)
        {
            deploys.erase(deploys.end());
            deploys.push_back(new_deploy);  //complete this deploy
        }
        if(vflavors_pridict_nums[i]){
            i++;  //to be continued...
        }
    }

    //OUTPUT
    string result_file = "";
    //vm nums
    int total_vm = accumulate(vflavors_pridict_nums_copy.begin(), vflavors_pridict_nums_copy.end(), 0);
    char s[20];
    sprintf(s, "%d", total_vm);
    result_file.append(s);
    result_file.append("\n");
    //vm items
    for(int i=0; i<vflavors.size(); i++)
    {
        char s[20];
        sprintf(s, "%d", vflavors_pridict_nums_copy[i]);
        result_file.append(vflavors[i]).append(" ").append(s).append("\n");
    }
    result_file.append("\n");

    //deploys
    sprintf(s, "%d", deploys.size());
    result_file.append(s).append("\n");
    for(int i=0; i<deploys.size(); i++)
    {
        char s[20];
        sprintf(s, "%d", deploys[i].phyid);
        result_file.append(s).append(" ");
        for(int j=0; j<deploys[i].vflavors.size(); j++)
        {
            if(deploys[i].nums[j]) //not 0
            {
                result_file.append(deploys[i].vflavors[j]).append(" ");
                sprintf(s, "%d", deploys[i].nums[j]);
                result_file.append(s).append(" ");
            }
        }
        result_file.append("\n");
    }

    cout << result_file;
    write_result(result_file.c_str(), filename);
}

