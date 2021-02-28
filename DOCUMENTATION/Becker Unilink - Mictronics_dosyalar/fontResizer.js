$(document).ready(function(){

   $("#default").click(function(event){
     	event.preventDefault();
	$("body").css("font-size","10px");
	$("div#right li,div#appendix li").css("background-position","0px 5px");
	$(".info-icon").css("background-position","0px 4px");
	$.cookie("reload","10px");
   });

   $("#larger").click(function(event){
     	event.preventDefault();
	$("body").css("font-size","12px");
	$("div#right li,div#appendix li").css("background-position","0px 7px");
	$(".info-icon").css("background-position","0px 6px");
	$.cookie("reload","12px");
   });

   $("#largest").click(function(event){
     	event.preventDefault();
	$("body").css("font-size","14px");
	$("div#right li,div#appendix li").css("background-position","0px 7px");
	$(".info-icon").css("background-position","0px 7px");
	$.cookie("reload","14px");
   });

// Cookie
var reload = $.cookie("reload");

// Calling cookie
if(reload == "10px") {
	$("body").css("font-size","10px");
	$("div#right li,div#appendix li").css("background-position","0px 5px");
	$(".info-icon").css("background-position","0px 4px");
};

if(reload == "12px") {
	$("body").css("font-size","12px");
	$("div#right li,div#appendix li").css("background-position","0px 7px");
	$(".info-icon").css("background-position","0px 6px");
};

if(reload == "14px") {
	$("body").css("font-size","14px");
	$("div#right li,div#appendix li").css("background-position","0px 7px");
	$(".info-icon").css("background-position","0px 7px");
};
});
