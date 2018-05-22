#include "RoomFactoryImpl.h"
#include "RoomImpl.h"

RoomFactoryImpl::RoomFactoryImpl(){
//    this->adapter = adapter;
      ic = Ice::initialize();
}
RoomPrx RoomFactoryImpl::createRoom(const string& name, const Ice::Current&){

    cout << "createRoom in RoomFactoryPrx triggered" << endl;
    adapter = ic->createObjectAdapterWithEndpoints("Room" + name, "default -p 10002");
    ServerPrx serverPrx = ServerPrx::uncheckedCast(adapter->addWithUUID(this));
    RoomPtr room = new RoomImpl(name, serverPrx);

//    ServerPtr server = new ServerImpl(adapter);
//    adapter->add(server, ic->stringToIdentity("Server"));
    adapter->activate();
//    adapter->add(room, ic->stringToIdentity("Room "+name));
    RoomPrx roomPrx = server->CreateRoom(name);
//    serverPrx->addRoom(roomPrx);
//    roomList.push_back(roomPrx);
    cout << "Created "<<name<<endl;

    return roomPrx;
}

RoomList RoomFactoryImpl::getRooms(const Ice::Current&){
    adapter = ic->createObjectAdapterWithEndpoints("Room", "default -p 10002");
    ServerPrx serverPrx = ServerPrx::uncheckedCast(adapter->addWithUUID(this));
    return serverPrx->getRooms();
}
void RoomFactoryImpl::addAdapter(const Ice::ObjectAdapterPtr& adapter){
    this->adapter = adapter;
}