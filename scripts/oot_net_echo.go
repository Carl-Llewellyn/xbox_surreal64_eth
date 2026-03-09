package main

import (
	"encoding/binary"
	"flag"
	"fmt"
	"net"
	"time"
)

const (
	packetBytes = 32
	wordCount   = packetBytes / 4
)

func decodeWordsBE(b []byte) [wordCount]uint32 {
	var words [wordCount]uint32
	for i := 0; i < wordCount; i++ {
		words[i] = binary.BigEndian.Uint32(b[i*4 : i*4+4])
	}
	return words
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
		if n < packetBytes {
			fmt.Printf("short packet from=%s size=%d (need %d)\n", addr.String(), n, packetBytes)
			continue
		}

		var p [packetBytes]byte
		copy(p[:], buf[:packetBytes])
		words := decodeWordsBE(p[:])
		rxCount++

		fmt.Printf(
			"rx#%d from=%s w0=%08X w1=%08X w2=%08X w3=%08X w4=%08X w5=%08X w6=%08X w7=%08X\n",
			rxCount, addr.String(),
			words[0], words[1], words[2], words[3], words[4], words[5], words[6], words[7],
		)

		if *echoBack {
			if _, err := conn.WriteTo(p[:], addr); err != nil {
				fmt.Printf("write error: %v\n", err)
			}
		}

		if time.Since(lastStat) >= 5*time.Second {
			fmt.Printf("stats: rx_total=%d\n", rxCount)
			lastStat = time.Now()
		}
	}
}
