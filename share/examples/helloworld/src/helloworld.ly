
import from "//std/system" ...

val port: Port = acquire(`dev.zuri.proto:fathom-elements-1`)
val root: Element = Element{ns = `dev.zuri.ns:io.fathomdata:richtext-1`, id = 29, "Hello, world"}
port.send(ReplaceOperation{path = "/", value = root})
