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

open class EnumAsLongSerializer<T : Enum<*>>(serialName: String, val serialize: (v: T) -> Long, val deserialize: (v: Long) -> T) : KSerializer<T> {
    override val descriptor: SerialDescriptor = PrimitiveSerialDescriptor(serialName, PrimitiveKind.LONG)

    override fun serialize(encoder: Encoder, value: T) {
        encoder.encodeLong(serialize(value))
    }

    override fun deserialize(decoder: Decoder) =
        deserialize(decoder.decodeLong())
}
