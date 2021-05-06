package wmon

import (
	"flag"
	"fmt"
	"log"
	"os"
	"testing"
)

var (
	devAddr    = flag.String("device_addr", "6F:6F:6F:6F:01", "The esb device pipeline address")
	serverAddr = flag.String("server_addr", "localhost", "The server address")
	serverPort = flag.Uint("server_port", 9815, "The server port")
)

func setup() {
	err := Open(*devAddr, *serverAddr, *serverPort)
	if err != nil {
		log.Fatalf("Setup: Connection Error: %v", err)
	}
}
func teardown() {
	err := Close()
	if err != nil {
		fmt.Printf("Error while disconnection: %v)", err)

	}
}

// TestGetFwVersion tests reading the FW version of the ESB device
func TestGetFwVersion(t *testing.T) {

	ver, err := GetFwVersion()

	if err != nil {
		t.Fatalf("%v", err)
	}

	fmt.Printf("Got FW Version: %v\n", ver)
}

// TestGetRawAdc tests reading raw ADC value
func TestGetRawAdc(t *testing.T) {

	adc, err := GetRawAdc()

	if err != nil {
		t.Fatalf("%v", err)
	}

	fmt.Printf("Got ADC value: %v\n", adc)
}

// TestGetThresholds tests reading ADC thresholds
func TestGetThresholds(t *testing.T) {

	low, high, err := GetThresholds()

	if err != nil {
		t.Fatalf("%v", err)
	}

	fmt.Printf("Got Thresholds: low %v  high %v\n", low, high)
}

// TestSetThresholds tests writing ADC thresholds
func TestSetThresholds(t *testing.T) {

	err := SetThresholds(2500, 5520)
	if err != nil {
		t.Fatalf("%v", err)
	}
}

// TestSaveConfig issues the command for saving the configuration persistently on the device
func TestSaveConfig(t *testing.T) {

	err := SaveConfig()
	if err != nil {
		t.Fatalf("%v", err)
	}
}

func TestGetState(t *testing.T) {

	state, err := GetState()

	if err != nil {
		t.Fatalf("%v", err)
	}

	fmt.Printf("Got State: %v\n", state)
}

func TestMain(m *testing.M) {
	setup()
	code := m.Run()
	teardown()
	os.Exit(code)
}
