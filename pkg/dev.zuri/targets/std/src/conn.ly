
@@Plugin("/std")

import from "/system" { Future }
import from "/url" { Url }


@AllocatorTrap("STD_CONN_CONNECTION_ALLOC")
defclass Connection sealed {

    init() {}

    def SendRaw(bytes: Bytes): Status {
        @{
            LoadData(bytes)
            Trap("STD_CONN_CONNECTION_SEND")
            PushResult(typeof Status)
        }
    }
}


defclass SenderConnection[SendType] final from Connection {

}


def ConnectSender[SendType](
    protocol: Protocol[SendType,Undef],
    endpoint: Url = {}
    ): Future[SenderConnection[SendType]]
{
    val fut = Future[SenderConnection[SendType]]{}
    val conn = SenderConnection[SendType]{}
    @{
        LoadData(fut)
        LoadData(conn)
        Trap("STD_CONN_MAKE_CONNECTION")
    }
    fut
}