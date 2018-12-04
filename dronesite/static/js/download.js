function selectall(el) {
	var files = $('#datafiles .checkbox input');
	for (var i=0; i<files.length; i++) {
		files[i].checked = el.checked;
	}
}

function remove() {
	$('#datafiles #file').each(function() {
		var e = $(this);
		if (e.find('.checkbox input')[0].checked) {
			var file = e.attr('file');
			$.post('../datafiles/remove/'+file);
			e.remove();
		}
	});
}

function download() {
	$('#download').empty();
	var files = $.find('#datafiles #file');
	for (var i=0; i<files.length; i++) {
		var e = $(files[i]);
		if (e.find('.checkbox input')[0].checked) {
			var file = e.attr('file');
			var a = $('<a/>');
			a.attr('href', '../datafiles/download/'+file);
			a.attr('class', 'todownload');
			a.attr('display', 'none');
			$('#download').append(a);
		}
	}
	$('#download .todownload').multiDownload({delay:50});
}
