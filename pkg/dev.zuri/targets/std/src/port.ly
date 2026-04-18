
@@Plugin("/system")

import from "/system" { Future }

defconcept Dump[InType, OutType] {

    decl Dump(i: InType): OutType | Error
}

defconcept Load[InType, OutType] {

    decl Load(o: OutType): InType | Error
}

defconcept RoundTrip[InType, OutType] {

    decl Dump(i: InType): OutType | Error

    decl Load(o: OutType): InType | Error
}

/**
 *
 */
@AllocatorTrap("STD_PORT_CONNECTION_ALLOC")
defclass Connection[SendType, RecvType] final {

    val _send: Serde[SendType, Bytes]
    val _recv: Serde[RecvType, Bytes]

    init() {
        @{
            Trap("STD_PORT_CONNECTION_CTOR")
        }
    }

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
def ConnectLocal[S,R](protocol: Protocol[S,R]): Future[Connection[S,R]] {

}