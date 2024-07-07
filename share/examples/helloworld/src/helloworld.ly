
import from "//std/system" ...

val port: Port = Acquire(`dev.zuri.proto:fathom-elements-1`)
val root: Element = Element{`dev.zuri.ns:io.fathomdata:richtext-1`, 29, "Hello, world"}
port.Send(ReplaceOperation{"/", root})
