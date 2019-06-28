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

    var data = {
        change: 1,
		perc: 100
    };
    
	console.log(endpoint);
	console.log(data);
    
    $.post(endpoint + '/postpenalty/', data, 'json').done(function(response) {
			console.log('Request Done: ' + endpoint + '/postpenalty/, \nres: ' + response);
			console.log(data);
        }).fail(function(textStatus, error) {
            alert(textStatus.responseText);
            console.log('Request Failed: ' + textStatus.responseText + ', ' + textStatus.status);
        });
}