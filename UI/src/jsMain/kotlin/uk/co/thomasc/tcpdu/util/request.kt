package uk.co.thomasc.tcpdu.util

import js.objects.jso
import kotlinx.serialization.KSerializer
import kotlinx.serialization.descriptors.PrimitiveKind
import kotlinx.serialization.descriptors.PrimitiveSerialDescriptor
import kotlinx.serialization.descriptors.SerialDescriptor
import kotlinx.serialization.encoding.Decoder
import kotlinx.serialization.encoding.Encoder
import react.router.NavigateOptions

val NewNavOption = jso<NavigateOptions> {
    state = false
}

open class EnumAsIntSerializer<T : Enum<*>>(serialName: String, val serialize: (v: T) -> Int, val deserialize: (v: Int) -> T) : KSerializer<T> {
    override val descriptor: SerialDescriptor = PrimitiveSerialDescriptor(serialName, PrimitiveKind.INT)

    override fun serialize(encoder: Encoder, value: T) {
        encoder.encodeInt(serialize(value))
    }

    override fun deserialize(decoder: Decoder) =
        deserialize(decoder.decodeInt())
}
