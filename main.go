package main

import (
	"fmt"
	"io"
	"os"
	"encoding/xml"
	_ "net/http"
)

type entry struct {
	Title string `xml:"title"`
	VideoId string `xml:"videoId"`
	Description string `xml:"group>description"`
}

func (e entry) String() string {
	return fmt.Sprintf("Title: %v\nURL: https://www.youtube.com/watch?v=%v\nDescription: %v\n", e.Title, e.VideoId, e.Description)
}

type youtube struct {
	Title string `xml:"title"`
	Entries []entry `xml:"entry"`
}

func (y youtube) String() string {
	var str string
	str = fmt.Sprintf("Title: %v\n\n", y.Title)
	for _, entry := range y.Entries {
		str += entry.String() + "------------------------\n"
	}
	return str
}

func main(){

	// resp, _ := http.Get("https://www.youtube.com/feeds/videos.xml?channel_id=UCrqM0Ym_NbK1fqeQG2VIohg")

	// bodyBytes, _ := io.ReadAll(resp.Body)

	// fmt.Println(string(bodyBytes))
	f, _ := os.Open("test.xml")
	defer f.Close()
	bodyBytes, _ := io.ReadAll(f)
//	fmt.Println(string(bodyBytes))
	var yt youtube
	fmt.Println(xml.Unmarshal(bodyBytes, &yt))
	fmt.Printf("%+v\n", yt)
}


