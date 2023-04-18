package main

import (
	"encoding/json"
	"encoding/xml"
	"fmt"
	"io"
	"net/http"
	_ "net/http"
	"os"
	"regexp"
	"strings"
	"time"
)

const (
	feedURL string = "https://www.youtube.com/feeds/videos.xml?channel_id="
	channelURL string = "https://www.youtube.com/@"
	followJsonFilePath = "follow.json"
)

type channelName string

func warn(format string, v ...interface{}) (int, error) {
	return fmt.Fprintf(os.Stdout, "[WARN]: " + format, v...)
}

func info(format string, v ...interface{}) (int, error) {
	return fmt.Fprintf(os.Stdout, "[INFO]: " + format, v...)
}

type video struct {
	ChannelId string `xml:"channelId" json:"channelId"`
	Title string `xml:"title" json:"title"`
	VideoId string `xml:"videoId" json:"url"`
	Published time.Time `xml:"published" json:"published"`
	Updated time.Time `xml:"updated" json:"update"`
}

func (e video) String() string {
	return fmt.Sprintf("%-10v %v\n%-10v %v\n%-10v %v\n%-10v %v\n%-10v %v\n", 
		"ChannelId:", e.ChannelId,
		"Title:", e.Title, 
		"URL:", e.VideoId, 
		"Published:", e.Published,
		"Updated:", e.Updated,
	)
}

type channel struct {
	Title string `xml:"title"`
	Videos []video `xml:"entry"`
}

func (c channel) String() string {
	var str string
	for _, video := range c.Videos {
		str += fmt.Sprintf("%-10v %v\n", "Channel:", c.Title)
		str += video.String() + "------------------------\n"
	}
	return str
}

func parseFeed(channelFeedURL string) (*channel, channelName,error) {


	resp, err := http.Get(channelFeedURL)
	if err != nil {
		return nil,channelName(""), err
	}

	bodyBytes, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, channelName(""),err
	}

// 	// for dev purpose, so no handling error v
// 	f, _ := os.Open("test.xml")
// 	defer f.Close()
// 	bodyBytes, _ := io.ReadAll(f)
// 	// for dev purpose, so no handling error ^

	var yt channel
	if err := xml.Unmarshal(bodyBytes, &yt); err != nil {
		return nil, channelName(""),err
	}
	var name channelName
	for i, video := range yt.Videos {
		if name == channelName("") {
			name = channelName(yt.Title)
		}
		yt.Videos[i].VideoId = fmt.Sprintf("https://www.youtube.com/watch?v=%v", video.VideoId)
		yt.Videos[i].ChannelId = channelFeedURL
	}
	
	return &yt, name,nil
}

func writeJson(yts map[channelName][]*channel) {
	jsonBytes, err := json.Marshal(yts)
	if err != nil {
		warn("Failed to marshal data: %v\n", err)
		return
	}
	f, err := os.OpenFile(followJsonFilePath, os.O_CREATE |  os.O_APPEND | os.O_WRONLY, 0755)
	if err != nil {
		warn("Failed to open file because of: %v\n", err)
		return
	}
	defer f.Close()
	n, err := io.WriteString(f,string(jsonBytes))
	if err != nil {
		warn("Failed to write follow data because of: %v\n", err)
	}
	info("Wrote %v bytes to json", n)
}

func parseJson() (map[channelName][]*channel, error) {
	var yts map[channelName][]*channel
	f, err := os.Open(followJsonFilePath)
	if err != nil {
		return yts, err
	}
	defer f.Close()
	jsonBytes, err := io.ReadAll(f)
	if err != nil {
		return yts, err
	}
	err = json.Unmarshal(jsonBytes, &yts)
	return yts, err
}


// TODO: compare new entries with existing
// TODO: handle new channel ids
// TODO: handle existing channel ids
// TODO: user interface
// TODO: periodic grab new data (perhaps could be delegated to cron instead)
// TODO: refactor

func main(){
	chanName := "Tsoding Daily"
	uniformChanName := strings.ToLower(strings.ReplaceAll(chanName, " ", ""))
	resp, err := http.Get(channelURL+uniformChanName)
	if err != nil {
		warn("Failed to get channel info")
		return
	}

	bodyBytes, err := io.ReadAll(resp.Body)
	if err != nil {
		warn("Failed to read response body")
		return
	}

// 	fmt.Println(string(bodyBytes))

	idRegexp := "\\w{8}_\\w{15}"
	re := regexp.MustCompile(strings.ReplaceAll(feedURL,"?", "\\?")+idRegexp)
	feedURLBytes := re.Find(bodyBytes)
	
// 	reId := regexp.MustCompile(idRegexp)
// 	fmt.Println(string(reId.Find(feedURLBytes)))

//	fmt.Println(parseJson())
	
	yt, name,_ := parseFeed(string(feedURLBytes))
	fmt.Printf("%+v\n", *yt)
	
	var yts []*channel
	yts = append(yts, yt)
	
	follow := make(map[channelName][]*channel)
	follow[name] = yts
	
	writeJson(follow)
}


