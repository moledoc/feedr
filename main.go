package main

import (
	"fmt"
	"io"
	"os"
	"encoding/xml"
	"encoding/json"
	"time"
	_ "net/http"
)

const (
	ytUrlBase string = "https://www.channel.com/feeds/videos.xml?channel_id="
)

type channelName string

func warn(format string, v ...interface{}) (int, error) {
	return fmt.Fprintf(os.Stdout, "[WARN]: " + format, v...)
}

type video struct {
	ChannelId string `xml:"channelId" json:"channelId"`
	Title string `xml:"title" json:"title"`
	VideoId string `xml:"videoId" json:"url"`
	Published time.Time `xml:"published" json:"published"`
	Updated time.Time `xml:"updated" json:"update"`
}

func (e video) String() string {
	//return fmt.Sprintf("Title: %v\nURL: https://www.channel.com/watch?v=%v\nLink: \n", e.Title, e.VideoId)
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
	//str = fmt.Sprintf("Title: %v\n\n", c.Title)
	for _, video := range c.Videos {
		str += fmt.Sprintf("%-10v %v\n", "Channel:", c.Title)
		str += video.String() + "------------------------\n"
	}
	return str
}

func parseFeed(channelId string) (*channel, channelName,error) {


// 	resp, err := http.Get(ytUrlBase+channelId)
// 	if err != nil {
// 		return nil,channelName(""), err
// 	}

	// bodyBytes, err := io.ReadAll(resp.Body)
// 	if err != nil {
// 		return nil, channelName(""),err
// 	}

	// for dev purpose, so no handling error v
	f, _ := os.Open("test.xml")
	defer f.Close()
	bodyBytes, _ := io.ReadAll(f)
	// for dev purpose, so no handling error ^

	var yt channel
	if err := xml.Unmarshal(bodyBytes, &yt); err != nil {
		return nil, channelName(""),err
	}
	var name channelName
	for i, video := range yt.Videos {
		if name == channelName("") {
			name = channelName(yt.Title)
		}
		yt.Videos[i].VideoId = fmt.Sprintf("https://www.channel.com/watch?v=%v", video.VideoId)
		yt.Videos[i].ChannelId = fmt.Sprintf(ytUrlBase+video.ChannelId)
	}
	
	return &yt, name,nil
}

func write(yts map[channelName][]*channel) {
	jsonBytes, err := json.Marshal(yts)
	if err != nil {
		warn("Failed to write channels because of: %v", err)
	}
	// TODO: write json file
	fmt.Println(string(jsonBytes),err)
	
	var tmp map[channelName][]*channel
	json.Unmarshal(jsonBytes, &tmp)
	fmt.Println(tmp)
}

// TODO: read existing json
// TODO: compare new entries with existing
// TODO: handle new channel ids
// TODO: handle existing channel ids
// TODO: user interface
// TODO: periodic grab new data (perhaps could be delegated to cron instead)

func main(){
	yt, name,_ := parseFeed("")
	fmt.Printf("%+v\n", *yt)
	
	var yts []*channel
	yts = append(yts, yt)
	
	follow := make(map[channelName][]*channel)
	follow[name] = yts
	
	write(follow)

}


