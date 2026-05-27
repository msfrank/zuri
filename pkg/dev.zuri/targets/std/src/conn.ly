
@@Plugin("/std")

import from "/system" { Future }
import from "/url" { Url }


@AllocatorTrap("STD_CONN_CONNECTION_ALLOC")
defclass Connection sealed {

    init() {}

    def _SendRaw(bytes: Bytes): Status {
        @{
            LoadData(bytes)
            Trap("STD_CONN_CONNECTION_SEND")
            PushResult(typeof Status)
        }
    }

    def _ReceiveRaw(): Future[Bytes] {
        val fut = Future[Bytes]{}
        @{
            LoadData(fut)
            Trap("STD_CONN_CONNECTION_RECEIVE")
        }
        fut
    }
}


defclass SenderConnection[SendType] final from Connection {

    def SendRaw(bytes: Bytes): Status {
        this._SendRaw(bytes)
    }
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


defclass ReceiverConnection[ReceiveType] final from Connection {

    def ReceiveRaw(): Future[Bytes] {
        this._ReceiveRaw()
    }
}


def ConnectReceiver[ReceiveType](
    protocol: Protocol[Undef,ReceiveType],
    endpoint: Url = {}
    ): Future[ReceiverConnection[ReceiveType]]
{
    val fut = Future[ReceiverConnection[ReceiveType]]{}
    val conn = ReceiverConnection[ReceiveType]{}
    @{
        LoadData(fut)
        LoadData(conn)
        Trap("STD_CONN_MAKE_CONNECTION")
    }
    fut
}