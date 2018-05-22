#include <Ice/Ice.h>
#include <Chat.h>
#include "Impl/RoomFactoryImpl.h"

using namespace std;
using namespace Chat;

class RoomFactoryHandler{
    private:
        Ice::CommunicatorPtr ic;
        ServerPrx serverPrx;
        RoomFactoryPrx roomFactoryPrx;

        int port;

        void connectToServer(){

            ic = Ice::initialize();
            Ice::ObjectPrx base = ic->stringToProxy(
                    "Server:default -p 10000");
            serverPrx = ServerPrx::checkedCast(base);
            if (!serverPrx)
                throw "Invalid proxy";

            RoomFactoryPtr roomFactoryPtr = new RoomFactoryImpl();
            Ice::ObjectAdapterPtr adapter = ic->createObjectAdapterWithEndpoints("RoomFactoryAdapter", "default -p 10001");
            adapter->add(roomFactoryPtr, ic->stringToIdentity("RoomFactory"));
            roomFactoryPrx = RoomFactoryPrx::uncheckedCast(adapter->addWithUUID(roomFactoryPtr));

            adapter->activate();

        }
    public:
        RoomFactoryHandler(){
            this->port = 10001;
            connectToServer();
        }
        ~RoomFactoryHandler(){
            //cos madrego
        }
        Ice::CommunicatorPtr getCommunicatorPtr(){
                return ic;
        }
        int getPort(){ return this->port;}
        RoomFactoryPrx getRoomFactoryPrx(){return this->roomFactoryPrx;}
        void createRoom(string name){
            roomFactoryPrx->createRoom(name);
        }
};
string commandSplitterGetCommand(string command){
    int position = 0;
    char current;
    string key;

    while(true){
        current = command[position];
        if(current == ' ' || position >= command.length()){
            break;
        }
        position++;
        key += current;
    }
    return key;
}
string commandSplitterGetArgs(string command){
    int position = 0;
    char current;
    string key, args;

    while(true){
        current = command[position];
        if(current == ' ' || position >= command.length()){
            break;
        }
        position++;
        key += current;
    }
    position++;

    for(int i = position; i < command.length(); i++){
        current = command[position++];
        args += current;
    }
    return args;
}
int main(int argc, char* argv[]){
    Ice::CommunicatorPtr ic;
    try{
        RoomFactoryHandler rfh;
        ic = rfh.getCommunicatorPtr();

        while(true){
            string userInput;
            getline(cin, userInput);

            string command, args;
            command = commandSplitterGetCommand(userInput);
            args = commandSplitterGetArgs(userInput);

            if(command == "/createroom"){
                cout << "gonna create room\n";
                rfh.createRoom(args);
            }else{
                cout << "invalid command. type /help for help\n";
            }
        }
    }catch(::IceUtil::NullHandleException& e){
        cout<<"[ERR]"<<endl;
        cout<<e<<endl;
    }

    if (ic)
        ic->destroy();
    return 0;
}