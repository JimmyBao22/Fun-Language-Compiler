fun val(a, b) {
    if (a == 0) {
        return b
    } else {
        b = b + 1
        return val(a-1, b)
    }
}

fun main() {
    print(val(8388608, 0))
}
