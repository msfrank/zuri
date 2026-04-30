
@@Plugin("/std")

import from "/system" { Future }


/**
 *
 */
@AllocatorTrap("STD_PORT_CONNECTION_ALLOC")
defclass Connection[SendType, RecvType] final {

    val _send: Serde[SendType, Bytes]
    val _recv: Serde[RecvType, Bytes]

    init() {}

    def SendRaw(bytes: Bytes): Status {
        @{
            LoadData(bytes)
            Trap("STD_PORT_CONNECTION_SEND")
            PushResult(typeof Status)
        }
    }

    def Send(payload: SendType): Status {
        val bytes: Bytes = expect this._send.Serialize(payload)
        SendRaw(bytes)
    }

    def ReceiveRaw(): Future[Bytes] {
        val fut: Future[Bytes] = Future[Bytes]{}
        @{
            LoadData(this._recv)
            Trap("STD_PORT_CONNECTION_RECEIVE")
        }
        fut
    }

    def WaitForReceive(): RecvType | Error {
        val bytes: Bytes = expect Await(ReceiveRaw())
    }
}

/**
 *
 */
def ConnectLocal[S,R](
    protocol: Protocol[S,R],
    endpoint: Url = {},
    using
    ): Future[Connection[S,R]] {

}