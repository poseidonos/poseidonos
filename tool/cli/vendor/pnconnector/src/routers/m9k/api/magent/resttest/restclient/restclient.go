package restclient

import (
	"bytes"
	"encoding/json"
	"net/http"
)

// Define an interface for Httpclient
type HTTPClient interface {
	Do(req *http.Request) (*http.Response, error)
}

var (
	Client HTTPClient
)

func init() {

	Client = &http.Client{}

}

// Send a post request to the specified URL with the body
func Post(url string, body interface{}, headers http.Header) (*http.Response, error) {
	jsonBytes, err := json.Marshal(body)
	if err != nil {
		return nil, err
	}
	// Make a POST request now 

	request, err := http.NewRequest(http.MethodPost, url, bytes.NewReader(jsonBytes))
	if err != nil {
		return nil, err
	}

	request.Header = headers
	return Client.Do(request)
}

// Send a get requests to the specified URL
func Get(url string, headers http.Header) (*http.Response, error) {

	request, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		return nil, err
	}

	if headers != nil {
		request.Header = headers
	}
	return Client.Do(request)

}
