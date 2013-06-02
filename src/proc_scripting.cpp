/* scripting */

static int proc_eval(Server *serv, Link *link, const Request &req, Response *resp){
  if(req.size() < 1){
    resp->push_back("client_error");
  }else{
    std::string val;
    int ret = serv->ssdb->eval(req[1], req, 2, &val);

    if(ret == 1){
      resp->push_back("ok");
      resp->push_back(val);
    }else if(ret == 0){
      resp->push_back("not_found");
    }else{
      log_error("script error: %s", val.c_str());
      resp->push_back("error");
      resp->push_back(val);
    }
  }
  return 0;
}
