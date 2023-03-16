//encoding UTF-8
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <unistd.h>
#include <ctime>



std::string exec(const char* cmd) //execute the command and catch the return value
{   
      char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}
    
std::vector<std::string> sort_mac(std::string string_to_sort) //Scrap the MAC Address from device frame
{
    std::vector<std::string> mac_container;
    std::vector<std::string> all;
    std::string line;
    std::istringstream iss(string_to_sort);
    
    while (std::getline(iss, line)) {
        std::istringstream iss(line);
        std::string cell;
        all.push_back(line);
        iss >> cell;
        if (cell == "Cell") {
            std::string device_number;
            iss >> device_number;
            std::string mac_address;
            for (int i = 0; i < 10; i++) {
                std::string all_the_addresses;
                iss >> all_the_addresses;
                
                mac_address += all_the_addresses;
            }
            mac_container.push_back(mac_address.substr(9, 26));
        }
    }
    return mac_container;  
}

void mac_addresses_e_write(std::vector<std::string> vector_to_write)    //Write the MAC address in exception.csv
{   std::ofstream file("exceptions.csv", std::ios_base::app);
    for (int i=0;i<vector_to_write.size();i++)
    {
        file << vector_to_write[i] <<","<<"\n";
    }
         
    file.close();
    
    
}

std::vector<std::string> mac_addresses_e_read()     //Read the address from exception.csv it' now the reference for say this address are around me 
{
    std::vector<std::string> excepts;
    std::fstream file;
    file.open("exceptions.csv");
    std::string line;


    if (!file.is_open()) {
        std::cout << "Unable to open file" << std::endl;
        
    }

    while (std::getline(file, line,'\n')) {
        std::istringstream templine(line);
        std::string data;
        
        while (std::getline(templine, data, ','))
        {
            excepts.push_back(data.c_str());
        }
    }

    file.close();
    return excepts;
}

std::vector<std::string> delete_same_values(std::vector<std::string> vector1, std::vector<std::string> vector2)     //Return the addresses who are not know by the exception.csv reference,
{                                                                                                                   //When a new address appear we have to check
                                                                                                                    //wich device is new in your environement then get it MAC addresse
   for (int i = 0; i < vector1.size(); i++) {
        for (int j = 0; j < vector2.size(); j++) {
            if (vector2[j] == vector1[i]) {
                
                vector2.erase(vector2.begin() + j);
                
                
            }
        }
    }
    return vector2;
}

int main(int argc, char** argv) {
    
    if (argc > 1) {
        
        std::string arg = argv[1];
        if (arg == "-d") {
            bool looper = true;
            while(looper)
            {
                std::vector<std::string> search_element = sort_mac(exec("sudo iwlist wlp2s0 scanning"));    //First time with -d arg we discover the environement and get all the addresse we want to excepts
                std::vector<std::string> read_for_compare = mac_addresses_e_read();
                std::vector<std::string> add_element = delete_same_values(read_for_compare, search_element);
                mac_addresses_e_write(add_element);
                sleep(5);
            }
            
        }
        
        if(arg == "-g")
        {   
           std::vector<std::string> search_element = sort_mac(exec("sudo iwlist wlp2s0 scanning"));     //Secondly with -g arg, we get the address who enter in your environement and can deduce MAC address of the devices
           std::vector<std::string> element_excepted = mac_addresses_e_read();
           std::vector<std::string> mac_finder = delete_same_values(element_excepted, search_element);
           for(std::string i : mac_finder)
           {
            std::cout<<i<<std::endl;
           }
        }

        if(arg == "-w")
        {
            std::string mac_to_research;
            std::cout<<"Enter the mac you want to detect : \n"<<std::endl;
            std::cin>>mac_to_research;
            std::cout<<"If "<<mac_to_research<<" is detected i will inform you."<<std::endl;
            for (std::string i : mac_addresses_e_read())
            {
                if (i == mac_to_research)
                {
                    std::cout<<mac_to_research<<" is already near of you"<<std::endl;
                }
            }
            while(1)
            {
                std::vector<std::string> search_element = sort_mac(exec("sudo iwlist wlp2s0 scanning"));
                for (std::string j : search_element)
                {
                    if (j == mac_to_research)
                    {   
                        time_t now = time(0);
                        std::cout<<"Find "<<mac_to_research<<" in your circle at timestamp : "<<now<<std::endl;
                    }
                }
                sleep(5);
            }
        }
        
    }   
    else 
    {
        std::cout << "Aucun argument spécifié." << std::endl;
    }
    return 0;
}
