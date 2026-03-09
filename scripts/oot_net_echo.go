package main

import (
	"encoding/binary"
	"flag"
	"fmt"
	"math"
	"net"
	"time"
)

const (
	metaBytes   = 32
	packetBytes = 64
	wordCount   = metaBytes / 4
	magicNSR1   = 0x4E535231
	magicNSR2   = 0x4E535232
)

func decodeWordsBE(b []byte) [wordCount]uint32 {
	var words [wordCount]uint32
	for i := 0; i < wordCount; i++ {
		words[i] = binary.BigEndian.Uint32(b[i*4 : i*4+4])
	}
	return words
}

func u32ToF32BE(v uint32) float32 {
	return math.Float32frombits(v)
}

func be16(b []byte) uint16 {
	return binary.BigEndian.Uint16(b)
}

func main() {
	listen := flag.String("listen", ":55365", "UDP listen address")
	echoBack := flag.Bool("echo", true, "echo packet back to sender")
	flag.Parse()

	conn, err := net.ListenPacket("udp4", *listen)
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	fmt.Printf("oot_net_echo listening on %s (packet=%d bytes)\n", *listen, packetBytes)

	buf := make([]byte, 2048)
	var rxCount uint64
	lastStat := time.Now()

	for {
		n, addr, err := conn.ReadFrom(buf)
		if err != nil {
			fmt.Printf("read error: %v\n", err)
			continue
		}
		if n < metaBytes {
			fmt.Printf("short packet from=%s size=%d (need at least %d)\n", addr.String(), n, metaBytes)
			continue
		}

		var p [packetBytes]byte
		copyLen := n
		if copyLen > packetBytes {
			copyLen = packetBytes
		}
		copy(p[:], buf[:copyLen])
		words := decodeWordsBE(p[:metaBytes])
		rxCount++

		if words[0] == magicNSR2 {
			seq := words[1]
			op := uint8(words[2] >> 24)
			rt := uint8(words[2] >> 16)
			addrLow := uint16(words[2] & 0xFFFF)
			cartAddr := words[3]
			value := words[4]
			pc := words[5]
			drops := words[6]
			payloadLen := words[7]
			if payloadLen > 32 {
				payloadLen = 32
			}
			if int(metaBytes+payloadLen) > copyLen {
				payloadLen = uint32(copyLen - metaBytes)
			}
			payload := p[metaBytes : metaBytes+payloadLen]

			fmt.Printf(
				"rx#%d from=%s nsr2 seq=%d op=0x%02X rt=0x%02X addr=%08X (low=%04X) value=%08X pc=%08X drops=%d payload_len=%d\n",
				rxCount, addr.String(), seq, op, rt, cartAddr, addrLow, value, pc, drops, payloadLen,
			)

			// PI DMA probe event from dma.c interceptor.
			// value currently carries DMA length and payload carries copied DMA bytes.
			if op == 0x31 {
				if payloadLen >= 30 && payload[0] == 'O' && payload[1] == 'O' && payload[2] == 'T' {
					xBits := binary.BigEndian.Uint32(payload[4:8])
					yBits := binary.BigEndian.Uint32(payload[8:12])
					zBits := binary.BigEndian.Uint32(payload[12:16])
					x := u32ToF32BE(xBits)
					y := u32ToF32BE(yBits)
					z := u32ToF32BE(zBits)
					pitch := int16(be16(payload[16:18]))
					yaw := int16(be16(payload[18:20]))
					roll := int16(be16(payload[20:22]))
					camYaw := int16(be16(payload[22:24]))
					buttons := be16(payload[24:26])
					stickX := int8(payload[26])
					stickY := int8(payload[27])
					level := payload[28]
					playerID := payload[3]
					fmt.Printf("  pi_dma_probe cart=%08X len=%d hdr=%c%c%c pid=%d\n",
						cartAddr, value, payload[0], payload[1], payload[2], playerID)
					fmt.Printf("    pos: x=%f y=%f z=%f (bits %08X %08X %08X)\n", x, y, z, xBits, yBits, zBits)
					fmt.Printf("    rot: pitch=%d yaw=%d roll=%d cam_yaw=%d\n", pitch, yaw, roll, camYaw)
					fmt.Printf("    in : buttons=%04X stick_x=%d stick_y=%d level=%d reserved=%d\n",
						buttons, stickX, stickY, level, payload[29])
				} else {
					fmt.Printf("  pi_dma_probe cart=%08X len=%d payload=% X\n", cartAddr, value, payload)
				}
			}
		} else if words[0] == magicNSR1 {
			seq := words[1]
			op := uint8(words[2] >> 24)
			rt := uint8(words[2] >> 16)
			addrLow := uint16(words[2] & 0xFFFF)
			cartAddr := words[3]
			value := words[4]
			pc := words[5]
			drops := words[6]
			fmt.Printf(
				"rx#%d from=%s nsr1 seq=%d op=0x%02X rt=0x%02X addr=%08X (low=%04X) value=%08X pc=%08X drops=%d\n",
				rxCount, addr.String(), seq, op, rt, cartAddr, addrLow, value, pc, drops,
			)
		} else if p[0] == 'O' && p[1] == 'O' && p[2] == 'T' {
			xBits := binary.BigEndian.Uint32(p[4:8])
			x := u32ToF32BE(xBits)
			fmt.Printf(
				"rx#%d from=%s raw_oot hdr=%c%c%c x_bits=%08X x=%f\n",
				rxCount, addr.String(), p[0], p[1], p[2], xBits, x,
			)
		} else {
			fmt.Printf(
				"rx#%d from=%s unknown w0=%08X w1=%08X w2=%08X w3=%08X w4=%08X w5=%08X w6=%08X w7=%08X\n",
				rxCount, addr.String(),
				words[0], words[1], words[2], words[3], words[4], words[5], words[6], words[7],
			)
		}

		if *echoBack {
			if _, err := conn.WriteTo(p[:copyLen], addr); err != nil {
				fmt.Printf("write error: %v\n", err)
			}
		}

		if time.Since(lastStat) >= 5*time.Second {
			fmt.Printf("stats: rx_total=%d\n", rxCount)
			lastStat = time.Now()
		}
	}
}
