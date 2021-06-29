package mocks

import "net/http"

// Struct for defining a mock client 
type MockClient struct {
	DoFunc func(req *http.Request) (*http.Response, error)
}

var (

	// This fetches the mock client's Do function
	GetDoFunc func(req *http.Request) (*http.Response, error)
)

// Define Do function which is client's Do func
func (m *MockClient) Do(req *http.Request) (*http.Response, error) {
	return GetDoFunc(req)
}
