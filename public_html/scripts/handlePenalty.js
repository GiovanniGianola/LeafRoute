console.log("Loading Handle Penalty");

$(document).ready(function(){
    // HANDLE ON APPLY PENALTY
	$("#applyPenalty").on("click", function() {
        console.log('apply penalty clicked');
        postPenalty();
   	});
});

// -------------- REQUESTS ---------------

function postPenalty(){

    var data = JSON.stringify({

    });
    
	console.log(endpoint);
    
    $.post(endpoint, data, function(data, status){
        alert("Data: " + data + "\nStatus: " + status);
    });
}