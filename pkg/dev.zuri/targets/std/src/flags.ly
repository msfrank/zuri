
defconcept IntoFlags[T] {

    decl ToValue(element: T): U32
}

defclass Flags[T] {

    val _intoFlags: IntoFlags[T]
    var _bits: U32

    init(using intoFlags: IntoFlags[T], initialSet: ...T) {
        this._intoFlags = intoFlags
        this._bits = 0 as U32
        for element: T in initialSet {
            this.Set(element)
        }
    }

    init _Copy(intoFlags: IntoFlags[T], bits: U32) {
        this._intoFlags = intoFlags
        this._bits = bits
    }

    def Set(element: T) {
        val value: U32 = this._intoFlags.ToValue[T](element)
        @{
            LoadData(this._bits)
            LoadData(value)
            BitwiseOr()
            StoreData(this._bits)
        }
    }

    def Unset(element: T) {
        val value: U32 = this._intoFlags.ToValue[T](element)
        @{
            LoadData(this._bits)
            LoadData(value)
            BitwiseXor()
            StoreData(this._bits)
        }
    }

    def Contains(element: T): Bool {
        val value: U32 = this._intoFlags.ToValue[T](element)
        var present: U32 = 0 as U32
        @{
            LoadData(this._bits)
            LoadData(value)
            BitwiseAnd()
            StoreData(present)
        }
        val zero = 0 as U32
        present == zero then false else true
    }

    def ToInt(): U32 {
        this._bits
    }
}
