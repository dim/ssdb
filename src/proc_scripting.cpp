/* scripting */

static int proc_eval(Server *serv, Link *link, const Request &req, Response *resp){
  if(req.size() < 1){
    resp->push_back("client_error");
  }else{
    std::vector<std::string> list;
    int ret = serv->ssdb->eval(req[1], req, 2, &list);

    if(ret == 1){
      resp->push_back("ok");
      for(int i=0; i<list.size(); i++){
        resp->push_back(list[i]);
      }
    }else if(ret == 0){
      resp->push_back("not_found");
    }else{
      log_error("script error: %s", list[0].c_str());
      resp->push_back("error");
      resp->push_back(list[0]);
    }
  }
  return 0;
}