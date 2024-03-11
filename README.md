## ytf

`ytf` (youtube feed) is a cli tool to get youtube feed distraction free. You need `ytfd` running in the background for `ytf` to work. `ytf` is written in C.

```txt
ytf <action|channel>
ACTION:
	<channel>			get latest feed for a channel; if not subscribed to channel, fresh data is pulled from youtube
	help, -h, --help		print help
	health [message]		checks daemon health; if message given, daemon reflects it, otherwise responds with pre-set message
	subs				print list of subscribed channels
	search <channel>		search channel; partial search somewhat effective, search with typo not so much
	sub <channel>			subscribe to channel
	unsub <channel>			unsubscribe to channel
	fetch <channel>			get latest feed for a channel; if not subscribed to channel, fresh data is pulled from youtube
	get <channel>			get latest feed for a channel; if not subscribed to channel, fresh data is pulled from youtube
EXAMPLES:
	* ./ytf health
	* ./ytf health 'hello world'
	* ./ytf subs
	* ./ytf tsodingdaily
	* ./ytf search ThePrimea
	* ./ytf sub ThePrimeTimeagen
	* ./ytf unsub ThePrimeTimeagen
	* ./ytf fetch TsodingDaily
	* ./ytf get tsodingdaily
```

## ytfd

`ytfd` (youtube feed daemon) is handling the backend for `ytf`. `ytfd` is written in Go.

```txt
ytfd [-notify={true|false}] [-subs=/path/to/subs/file] [-refrate={minutes}] [-debug] 
  -debug
    	sets logging to stderr
  -help
    	print help
  -notify
    	creates dunstify notification when a new video for a subscribed channel is detected. Depends on dunstify. If dunstify is not detected in the system, internal flag value is set to false (default true)
  -refrate int
    	refresh rate in minutes, i.e. how often daemon checks youtube (default 15)
  -subs string
    	path to file that contains names of subscribed channels, one per each line

Examples:
	* ./ytfd -help
	* ./ytfd -subs=./example.subs -refrate=7 -notify
	* ./ytfd -subs=./example.subs -refrate=7 -notify=false -debug
```

## Author

Meelis Utt
