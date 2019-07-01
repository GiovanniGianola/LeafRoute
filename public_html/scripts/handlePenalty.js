console.log("Loading Handle Penalty");
var data = {
    change: 0,
    perc: 0
};

$(document).ready(function(){
    // HANDLE ON APPLY PENALTY
	$("#applyPenalty").on("click", function() {

        console.log('apply penalty clicked');
        perc = parseInt($('#percentage').val());
        console.log(perc);

        if(Number.isNaN(perc) || perc < 0 || typeof perc != 'number' || perc > 100){
            alert("Invalid input: 0<= perc <= 100");
            perc = 0;
            $('#percentage').val(0);
        }else{
            data.change = 1;
            data.perc = perc;
            postPenalty();
        }
   	});

	$("#defaultGraph").on("click", function() {
	    console.log('use default graph');
        data.change = 0;
	    postPenalty();
    })

});

// -------------- REQUESTS ---------------

function postPenalty(){
    
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
