function makeHTTPRequest(url, onReady, userData) {
    var request;
    var onReadyFunc = onReady;
    var userDataLocal = userData;
    if (window.XMLHttpRequest) {
        request = new XMLHttpRequest();
    } else if (window.ActiveXObject) {
        request = new ActiveXObject("Microsoft.XMLHTTP");
    }
    request.open('GET', url, true);
    request.onreadystatechange = function() {
        if (request.readyState != 4) return;
        onReadyFunc(request, request.status == '200', userDataLocal);
    }
    request.send(null);
    return request;
}

function makeURLParams(params) {
    var parts = new Array();
    var k,v;
    for (key in params) {
        parts.push(encodeURIComponent(key) + '=' + encodeURIComponent(params[key]));
    }
    return parts.join('&');
}

function makeURL(url, params) {
    return url + '?' + makeURLParams(params);
}

function getNodeContent(node) {
    var value = '';
    var child = node.firstChild;
    while (child != null) {
        value += child.nodeValue;
        child = child.nextSibling;
    }
    return value;
}

function getInnerSize(theDocument) {
    if (theDocument == null) theDocument = document;
    var x, y;
    if (theDocument.innerWidth) {
        x = theDocument.innerWidth;
        y = theDocument.innerHeight;
    } else if (theDocument.documentElement && theDocument.documentElement.clientHeight) {
        x = theDocument.documentElement.clientWidth;
        y = theDocument.documentElement.clientHeight;
    } else if (theDocument.body) {
        x = theDocument.clientWidth;
        y = theDocument.clientHeight;
    }
    return [x, y];
}

function stopEventPropagation(e)
{
    var ev = e;
    if (!e) ev = window.event;
    ev.cancelBubble = true;
    if (ev.stopPropagation) ev.stopPropagation();
}

function showMap(lat, lon, zoom)
{
    makeHTTPRequest(makeURL('/map/', {lat: lat, lon: lon, zoom: zoom}));
}
