package wmon

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"strings"

	"github.com/spritkopf/esb-bridge/pkg/client"
	"github.com/spritkopf/esb-bridge/pkg/esbbridge"
	binarysensor "github.com/spritkopf/esb-home/pkg/binary-sensor"
)

const (
	binarySensorChannel  = 0
	esbCmdGetVersion     = 0x10
	esbCmdGetADC         = 0x40
	esbCmdSetThresholds  = 0x81
	esbCmdGetThresholds  = 0x82
	esbCmdSetCentralAddr = 0x83
	esbCmdSaveConfig     = 0x84
)

// FirmwareVersion holds the version number of the esb device
type FirmwareVersion struct {
	major uint
	minor uint
	patch uint
}

func (v FirmwareVersion) String() string {
	return fmt.Sprintf("%v.%v.%v", v.major, v.minor, v.patch)
}

var esbClient client.EsbClient
var esbAddress []byte
var sensor binarysensor.BinarySensor

// Open opens a connection to the esb-bridge RPC server.
// Params:
//   - deviceAddress: ESB device pipeline address in format xx:xx:xx:xx:xx
//   - serverAddress: IP address of the esb-bridge server (e.g. "10.32.2.100", or "localhost")
//   - serverPort : Port of the esb-bridge server (e.g. 9815)
func Open(deviceAddress string, serverAddress string, serverPort uint) error {

	// decode target address to bytes
	addrBytes, err := hex.DecodeString(strings.ReplaceAll(deviceAddress, ":", ""))

	if err != nil {
		return fmt.Errorf("invalid format for deviceAddress: %v", err)
	}
	if len(addrBytes) != 5 {
		return fmt.Errorf("invalid length for deviceAddress: need 5, got %v", len(addrBytes))
	}
	esbAddress = addrBytes

	err = esbClient.Connect(fmt.Sprintf("%v:%v", serverAddress, serverPort))
	if err != nil {
		return fmt.Errorf("connection Error (esbClient): %v", err)
	}
	// connect binary sensor interface
	err = sensor.Open(deviceAddress, serverAddress, serverPort)
	if err != nil {
		return fmt.Errorf("connection Error (binary-sensor): %v", err)
	}

	return err
}

// Close closes the connection to the esb-bridge RPC server
func Close() error {
	err := esbClient.Disconnect()
	if err != nil {
		return fmt.Errorf("error closing connection to esbClient: %v", err)
	}

	err = sensor.Close()
	if err != nil {
		return fmt.Errorf("error closing connection to binarySensor: %v", err)
	}
	return err
}

// GetFwVersion reads the Firmware version from the washing monitor device
func GetFwVersion() (string, error) {

	answerMsg, err := esbClient.Transfer(esbbridge.EsbMessage{Address: esbAddress, Cmd: esbCmdGetVersion})

	if err != nil {
		return "", fmt.Errorf("ESB Transfer error: %v", err)
	}
	if answerMsg.Error != 0 {
		return "", fmt.Errorf("ESB answer has error: %v", answerMsg.Error)
	}
	version := fmt.Sprintf("%v.%v.%v", answerMsg.Payload[0], answerMsg.Payload[1], answerMsg.Payload[2])
	return version, nil
}

// GetRawAdc reads the current ADC value of the photoresitor
func GetRawAdc() (int16, error) {

	answerMsg, err := esbClient.Transfer(esbbridge.EsbMessage{Address: esbAddress, Cmd: esbCmdGetADC})

	if err != nil {
		return 0, fmt.Errorf("ESB Transfer error: %v", err)
	}
	if answerMsg.Error != 0 {
		return 0, fmt.Errorf("ESB answer has error: %v", answerMsg.Error)
	}
	adcVal := int16(binary.LittleEndian.Uint16(answerMsg.Payload[:2]))
	return adcVal, nil
}

// GetThresholds reads the High and Low thresholds of the device.
func GetThresholds() (low uint16, high uint16, err error) {

	answerMsg, err := esbClient.Transfer(esbbridge.EsbMessage{Address: esbAddress, Cmd: esbCmdGetThresholds})

	if err != nil {
		return 0, 0, fmt.Errorf("ESB Transfer error: %v", err)
	}
	if answerMsg.Error != 0 {
		return 0, 0, fmt.Errorf("ESB answer has error: %v", answerMsg.Error)
	}
	thresholdLow := binary.LittleEndian.Uint16(answerMsg.Payload[:2])
	thresholdHigh := binary.LittleEndian.Uint16(answerMsg.Payload[2:])
	return thresholdLow, thresholdHigh, nil
}

// SetThresholds sets the High and Low thresholds of the device. When the ADC value is below `low`,
// the binary sensor value will be false (true if ADC is above `high`)
func SetThresholds(low uint16, high uint16) error {

	buf := new(bytes.Buffer)

	err := binary.Write(buf, binary.LittleEndian, low)
	if err != nil {
		return fmt.Errorf("could not convert to bytes: %v", err)
	}
	err = binary.Write(buf, binary.LittleEndian, high)
	if err != nil {
		return fmt.Errorf("could not convert to bytes: %v", err)
	}
	answerMsg, err := esbClient.Transfer(esbbridge.EsbMessage{
		Address: esbAddress,
		Cmd:     esbCmdSetThresholds,
		Payload: buf.Bytes()})

	if err != nil {
		return fmt.Errorf("ESB Transfer error: %v", err)
	}
	if answerMsg.Error != 0 {
		return fmt.Errorf("ESB answer has error: %v", answerMsg.Error)
	}
	return nil
}

// SaveConfig stores the currently active configuration persistently on the device
func SaveConfig() (err error) {

	answerMsg, err := esbClient.Transfer(esbbridge.EsbMessage{Address: esbAddress, Cmd: esbCmdSaveConfig})

	if err != nil {
		return fmt.Errorf("ESB Transfer error: %v", err)
	}
	if answerMsg.Error != 0 {
		return fmt.Errorf("ESB answer has error: %v", answerMsg.Error)
	}

	return nil
}

// GetState reads the current state of the binary sensor channel the status LED is connected to
func GetState() (bool, error) {

	state, err := sensor.GetValue(binarySensorChannel)
	if err != nil {
		return false, fmt.Errorf("error getting channel value: %v", err)
	}

	return state, nil

}
