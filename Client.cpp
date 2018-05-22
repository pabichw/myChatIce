#include <Ice/Ice.h>
#include <Chat.h>
#include <time.h>
#include "Impl/UserImpl.h"

using namespace std;
using namespace Chat;

class ClientHandler{

private:
    Ice::CommunicatorPtr ic;
    ServerPrx serverPrx;
    RoomPrx roomPrx;
    UserPrx userPrx;
    string userName;
    string password;
    int port;

    void connectToServer(){
        this->port = randPort();
        ic = Ice::initialize();
        Ice::ObjectPrx base = ic->stringToProxy(
                "Server:default -p 10000");
        serverPrx = ServerPrx::checkedCast(base);
        if (!serverPrx)
            throw "Invalid proxy";
    }

    int randPort(){
        srand (time(NULL));
        return rand() % 1000 + 10000;
    }

    string sgetPort(){
        ostringstream ss;
        ss << port;
        return ss.str();
    }

public:
    ClientHandler(){
        connectToServer();
    }

    ~ClientHandler(){
        if(roomPrx){
            roomPrx->LeaveRoom(userPrx, password);
            serverPrx->removeUser(userPrx);
            ic->destroy();
        }
    }

    Ice::CommunicatorPtr getCommunicatorPtr(){
        return ic;
    }

    string getUserName(){
        return userName;
    }

    string getRoomName(){
        return roomPrx->getName();
    }

    bool isRoom(){
        if(roomPrx){
            return true;
        } else {
            return false;
        }
    }

    bool createUser(string userName, string password){
        try{
            serverPrx->FindUser(userName);
        } catch(NoSuchUserExists e){
            this->userName = userName;
            this->password = password;

            Ice::ObjectAdapterPtr adapter
                    = ic->createObjectAdapterWithEndpoints(userName, "default -p " + sgetPort());
            adapter->activate();
            UserPtr userPtr = new UserImpl(userName, password);
            userPrx = UserPrx::uncheckedCast(adapter->addWithUUID(userPtr));
            serverPrx->RegisterUser(userPrx);
            return true;
        }
        return false;
    }
    bool createRoom(string roomName){

        try{
            serverPrx->FindRoom(roomName);
        } catch(NoSuchRoomExists e){

            if(roomPrx){
                roomPrx->LeaveRoom(this->userPrx, this->password);
            }
            roomPrx = serverPrx->CreateRoom(roomName);
            roomPrx->AddUser(userPrx, this->password);
            return true;
        }
        return false;
    }
    bool joinRoom(string roomName){
        RoomPrx joinRoomPrx;

        try{
            joinRoomPrx = serverPrx->FindRoom(roomName);
        }catch(NoSuchRoomExists e){
            return false;
        }
        if(roomPrx){
            roomPrx->LeaveRoom(this->userPrx, this->password);
        }
        roomPrx = joinRoomPrx;
        try{
            roomPrx->AddUser(this->userPrx, this->password);
        }catch(::Ice::UnknownException e){
            cout << e << endl;
        }
        return true;
    }
    RoomList askForRoomList(){
        return serverPrx->getRooms();
    }
    void send(string message){
        roomPrx->SendMessage(userPrx, message, this->password);
    }

    void sendPriv(string userNameDestination, string message){
        try{
            UserPrx destinationUserPrx = serverPrx->FindUser(userNameDestination);
            destinationUserPrx->SendPrivateMessage(this->userPrx, message);
        }catch(NoSuchUserExists e){
            cout << "[ERR] User doesn't exists" << endl;
        }
    }
    void changePassword(string oldPassword, string newPassword){
        try{
            userPrx->ChangePassword(oldPassword, newPassword);
            cout << "Password has changed";
        }catch(WrongPassword e){
            cout << "Wrong password. Changing password has failed :-)\n";
        }
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
ClientHandler registerClient(ClientHandler client){
    bool isRegistered;
    string userName, password;

    do {
        cout << "--- Sing Up ---" << endl;
        cout << "User name: ";
        cin >> userName;
        cout << "Password: ";
        cin >> password;
        cout << endl;
            isRegistered = client.createUser(userName, password);
        if(!isRegistered){
            cout << "This nickname is already registered! Choose different one..." << endl;
        }else{
            cout << "You are now registered as " + client.getUserName() << endl;
        }
    } while(!isRegistered);

    return client;
}        

int
main(int argc, char* argv[])
{
    int status = 0;
    Ice::CommunicatorPtr ic;
    try {
        ClientHandler client;
        ic = client.getCommunicatorPtr();
        string userName, password;

        client = registerClient(client);

        string userInput;
        string command;
        string args;

        cin.ignore();
        do{
            getline(cin, userInput);

            if(userInput[0] == '/'){
                command = commandSplitterGetCommand(userInput);
                args = commandSplitterGetArgs(userInput);
                if(command == "/join"){
                    if(client.joinRoom(args)){
                        cout << "Joined room " << args << endl;
                    } else {
                        cout << "Can't join room. Check if room exists!" << endl;
                    }
                    continue;

                }else if(command == "/priv"){
                    string userNameDestination = args;
                    string message;
                    cout << "Priv to ["<< userNameDestination << "]:" << endl;
                    getline(cin, message);
                    client.sendPriv(userNameDestination, message);
                    continue;
                }else if(command == "/userlist"){
                    RoomList roomList = client.askForRoomList();
                    cout<<"Users: "<<endl;;
                    for(vector<RoomPrx>::iterator it = roomList.begin(); it != roomList.end(); ++it) {
                        cout << (*it)->getName() << endl;
                        UserList userList = (*it)->getUsers();
                        for(vector<UserPrx>::iterator u = userList.begin(); u != userList.end(); ++u){
                            cout <<"\t"<< (*u)->getName() << endl;
                        }
                    }
                }else if(command == "/password"){
                    string currPass, newPass;
                    cout << "Current password: ";
                    getline(cin, currPass);
                    cout << "New password: ";
                    getline(cin, newPass);
                    client.changePassword(currPass, newPass);
                }else if(command == "/help"){
                    cout << "/join: to join a room"<< endl;
                    cout << "/sendPriv: sending private message to specific user" << endl;
                    cout << "/password: to change your password" << endl;
                    cout << "/userlist: to display users in rooms" << endl;
                    cout << "/exit: the only way to exit program without crashing the server xdd" <<endl;
                    continue;
                }else if(command == "/exit"){
                    //do nothin
                }else{
                    cout << "Wrong command. Type /help for help" << endl;
                }
            }else{
                client.send(userInput);
            }
        }while(userInput != "/exit");


    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
    }
    if (ic)
        ic->destroy();

    cout << "Quiting server";
    return status;
}
