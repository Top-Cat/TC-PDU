package uk.co.thomasc.tcpdu.util

object CommonMath {
    fun fixed(n: Float, p: Int) = fixedStr(n, p).toFloat()
    fun fixed(n: Double, p: Int) = fixedStr(n, p).toDouble()

    fun fixedStr(n: Float, p: Int) = fixedStr(n.toDouble(), p)
    fun fixedStr(n: Double, p: Int) = intFixed(n, p) as String

    private fun intFixed(n: Double, p: Int) = n.asDynamic().toFixed(p)
}

fun Float.fixed(n: Int) = CommonMath.fixed(this, n)
fun Double.fixed(n: Int) = CommonMath.fixed(this, n)
