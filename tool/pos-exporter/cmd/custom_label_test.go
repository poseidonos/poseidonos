package cmd 

import (
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"strconv"
	"testing"
	"os/exec"
	"bytes"

	"github.com/prometheus/client_golang/prometheus/promhttp"
	pm "github.com/poseidonos/pos-exporter/api/pos.metric"
)

const (
	add_label_test_metric_cnt      = 5
	add_label_test_custom_label_key = "key"
	add_label_test_custom_label_val = "val"
	add_label_test_custom_label = add_label_test_custom_label_key + "=" + "\\\"" + add_label_test_custom_label_val + "\\\""
)

func TestCustomLabel(t *testing.T) {
	
	// Prepare exporter
	ts := httptest.NewServer(promhttp.Handler())
	defer ts.Close()

	// Set custom label
	setCustomLabel(add_label_test_custom_label_key, add_label_test_custom_label_val)
	t.Logf("Add custom label: %s", add_label_test_custom_label)

	// Add metric
	Request := make([]pm.MetricPublishRequest, add_label_test_metric_cnt)
	Metric := make([]pm.Metric, add_label_test_metric_cnt)

	for i := 0; i < add_label_test_metric_cnt; i++ {

		Metric[i].Type = pm.MetricTypes_COUNTER
		Metric[i].Name = "counter_" + strconv.Itoa(i)
		Metric[i].Labels = []*pm.Label{&pm.Label{Key:"k", Value:"v"}}
		Metric[i].Value = &pm.Metric_CounterValue{CounterValue: uint64(i)}

		Request[i].Metrics = []*pm.Metric{&Metric[i]}
		parseRequest(&(Request[i]))
		t.Logf("Add: %s", Metric[i].Name)
	}

	// Get the result from exporter
	res, _ := http.Get(ts.URL)
	mBody, _ := ioutil.ReadAll(res.Body)

	// Check Validity
	for i := 0; i < add_label_test_metric_cnt; i++ {

		var exported_data bytes.Buffer
		exported_data.WriteString(string(mBody))

		name := Metric[i].Name
		var output bytes.Buffer
		var err error

		cmd1 := exec.Command("grep", name)
		cmd2 := exec.Command("grep", "-e", add_label_test_custom_label)

		cmd1.Stdin = &exported_data
		cmd2.Stdin, err = cmd1.StdoutPipe()
		if err != nil {
			t.Logf("%s", err)
		}

		cmd2.Stdout = &output

		err = cmd2.Start()
		if err != nil {
			t.Errorf("%s", err)
			t.FailNow()
		}

		err = cmd1.Run()
		if err != nil {
			t.Errorf("%s", err)
			t.FailNow()
		}
		
		err = cmd2.Wait()
		if err != nil {
			t.Errorf("Metric(%s) contains no custom label", name)
			t.FailNow()			
		} else{			
			t.Logf("Metric(%s) contains a custom label (%s)", name, add_label_test_custom_label)			
		}			
	}
}
