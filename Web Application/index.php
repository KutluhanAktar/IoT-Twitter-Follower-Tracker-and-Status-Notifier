<?php

// Define the twitter_follower_tracker class and its functions:
class twitter_follower_tracker{
	public $account;
	
	public function __init__($account){
		$this->account = $account;
	}
	
	public function get_followers(){
		$url = "https://cdn.syndication.twimg.com/widgets/followbutton/info.json?screen_names=".$this->account;
		$data = json_decode(file_get_contents($url, true));
		echo "%".$data[0]->followers_count."%".$data[0]->name."%".$data[0]->id."%";
	}
}

// Define the new 'twitter' class object:
$twitter = new twitter_follower_tracker();
$twitter->__init__("ThAmplituhedron");

// Print information that of the given Twitter account.
$twitter->get_followers();

?>