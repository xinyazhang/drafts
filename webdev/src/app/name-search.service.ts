import { Injectable } from '@angular/core';
import { Headers, Http } from '@angular/http';
import { SearchResult, SearchReply } from './search-result';
import 'rxjs/add/operator/toPromise';

@Injectable()
export class NameSearchService {
	private serviceUrl = '/api/locate';
	private headers = new Headers({
			'Content-Type': 'application/json'});

	constructor(private http: Http) { }

	getMatchedFiles(regex:string, offset: number, num: number, cookie: string) : Promise<SearchReply> {
		let json = {
				cat : "byname",
				matcher : "regex",
				pattern : regex,
				start : offset,
				"number" : num
			};
		if (cookie != "")
			json["cache_cookie"] = cookie;
		let jsonstring = JSON.stringify(json);

		return this.http.post(this.serviceUrl, json, {headers: this.headers})
			.toPromise()
			.then(response => response.json())
			.catch(this.handleError);
	}
	
	private handleError(error: any) {
		console.error('An error occurred', error);
		return Promise.reject(error.message || error);
	}
}
