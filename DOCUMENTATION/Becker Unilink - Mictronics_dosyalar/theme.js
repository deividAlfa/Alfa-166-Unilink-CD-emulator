// Author:Devolux
// Author URI:http://devolux.org/

$(document).ready(function(){

//Superfish menu
$("ul.sf-menu").supersubs().superfish(
{
            delay:       1000,                            // one second delay on mouseout
            animation:   {opacity:'show'},  // fade-in and slide-down animation
            speed:       'normal',                          // faster animation speed
            autoArrows:  false,                           // disable generation of arrow mark-up
            dropShadows: false                            // disable drop shadows
        }
);

//Toggle functions
 $("#toggle-all").toggle(
                    function(){
                         $(".excerpt").hide('slow');
			 $("#toggle").attr("class","show-all");
                    }, function() {
                         $(".excerpt").show('slow');
			 $("#toggle").attr("class","hide-all");
                    });

$(".view-excerpt").click(function (event) {
	event.preventDefault();
      $(this).parents(".headline").next(".excerpt").toggle("normal");
    });

 });
