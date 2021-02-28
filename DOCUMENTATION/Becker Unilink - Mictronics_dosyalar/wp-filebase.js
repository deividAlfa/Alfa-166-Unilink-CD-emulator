function wpfilebase_filedetails(id) {
	var dtls = document.getElementById('wpfilebase-filedetails' + id);
	if(dtls)
		dtls.style.display = (dtls.style.display != 'none') ? 'none' : 'block';	
	return false;
}

function wpfilebase_catselect_changed(sel) {
	var url = wpfilebase_get_cat_url(sel.options[sel.selectedIndex].value);
	window.location.href = url;
}

function wpfilebase_get_cat_url(id) {
	if(typeof wpfb_cat_urls == 'undefined')
		return '';		
	for(var i = 0; i < wpfb_cat_urls.length; i++) {
		if(wpfb_cat_urls[i].id == id)
			return wpfb_cat_urls[i].url;
	}	
	return wpfb_cat_urls[0].url;
}