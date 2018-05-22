#ifndef ROOMFACTORYI_H
#define ROOMFACTORYI_H

#include <Ice/Ice.h>
#include <Chat.h>

using namespace std;
using namespace Chat;

class RoomFactoryImpl : public RoomFactory{
private:
    RoomList roomList;
    Ice::CommunicatorPtr ic;
    Ice::ObjectAdapterPtr adapter;
    ServerPrx serverPrx;
public:
    RoomFactoryImpl();
    virtual RoomPrx createRoom(const ::std::string& name, const ::Ice::Current& current);
    virtual RoomList getRooms(const ::Ice::Current& current);
    void addAdapter(const Ice::ObjectAdapterPtr& adapter);
};

#endif ROOMFACTORYI_H