

def Exists(pathOrString: Path | String): Bool {
    Access(exists = true)
}

def Access(
    pathOrString: Path | String,
    readAllowed: Bool = false,
    writeAllowed: Bool = false,
    executeAllowed: Bool = false,
    exists: Bool = false
): Bool {

}