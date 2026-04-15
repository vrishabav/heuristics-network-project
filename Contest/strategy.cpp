#include <bits/stdc++.h>
using namespace std;

struct Server { int id, z, c, r = -1, s = -1, p = -1; };

vector<int>          pool_total;   
vector<vector<int>>  pool_rack;    
vector<multiset<int>> rack_ms;     
vector<int>          pool_gp;      
multiset<int>        gp_ms;        


void move_to(Server& sv, int new_p) {
    int op = sv.p, c = sv.c, r = sv.r;

    gp_ms.erase(gp_ms.find(pool_gp[op]));
    gp_ms.erase(gp_ms.find(pool_gp[new_p]));

    pool_total[op] -= c;
    rack_ms[op].erase(rack_ms[op].find(pool_rack[op][r]));
    pool_rack[op][r] -= c;
    rack_ms[op].insert(pool_rack[op][r]);
    pool_gp[op] = pool_total[op] - *rack_ms[op].rbegin();   

    pool_total[new_p] += c;
    rack_ms[new_p].erase(rack_ms[new_p].find(pool_rack[new_p][r]));
    pool_rack[new_p][r] += c;
    rack_ms[new_p].insert(pool_rack[new_p][r]);
    pool_gp[new_p] = pool_total[new_p] - *rack_ms[new_p].rbegin();

    gp_ms.insert(pool_gp[op]);
    gp_ms.insert(pool_gp[new_p]);
    sv.p = new_p;
}


int main() {
    auto t_start = chrono::high_resolution_clock::now();
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int R, S, U, P, M;
    cin >> R >> S >> U >> P >> M;
    
    vector<set<pair<int,int>>> seg(R);
    vector<multiset<int>>      slen(R);
    
    auto seg_add = [&](int r, int l, int e) {
        if (l > e) return;
        seg[r].insert({l, e});
        slen[r].insert(e - l + 1);
    };
    
    auto seg_del = [&](int r, set<pair<int,int>>::iterator it) {
        slen[r].erase(slen[r].find(it->second - it->first + 1));
        seg[r].erase(it);
    };

    
    for (int r = 0; r < R; ++r) seg_add(r, 0, S - 1);
    
    for (int i = 0; i < U; ++i) {
        int ri, si;  cin >> ri >> si;
        
        auto it = seg[ri].upper_bound({si, INT_MAX});
        if (it == seg[ri].begin()) continue;
        --it;
        if (si > it->second) continue;   
        int l = it->first, e = it->second;
        seg_del(ri, it);
        seg_add(ri, l, si - 1);   
        seg_add(ri, si + 1, e);   
    }

    vector<Server> servers(M);
    for (int i = 0; i < M; ++i) { servers[i].id = i; cin >> servers[i].z >> servers[i].c; }
    
    vector<int> ord(M);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return servers[a].c > servers[b].c; });  
    
    int rack_cur = 0;
    for (int idx : ord) {
        auto& sv = servers[idx];
        int z = sv.z;
        for (int d = 0; d < R; ++d) {
            int r = (rack_cur + d) % R;
            
            if (slen[r].empty() || *slen[r].rbegin() < z) continue;
            
            for (auto it = seg[r].begin(); it != seg[r].end(); ++it) {
                if (it->second - it->first + 1 >= z) {
                    int l = it->first, e = it->second;
                    sv.r = r;  sv.s = l;
                    seg_del(r, it);
                    seg_add(r, l + z, e);   
                    rack_cur = (r + 1) % R; 
                    break;
                }
            }
            if (sv.r != -1) break;
        }
    }
    
    { vector<set<pair<int,int>>>().swap(seg);  }
    { vector<multiset<int>>().swap(slen); }
    
    pool_total.assign(P, 0);
    pool_rack.assign(P, vector<int>(R, 0));
    rack_ms.resize(P);
    pool_gp.resize(P);
   
    for (int p = 0; p < P; ++p)
        for (int r = 0; r < R; ++r)
            rack_ms[p].insert(0);
   
    vector<vector<int>> rack_srvs(R);
    for (int i = 0; i < M; ++i)
        if (servers[i].r != -1) rack_srvs[servers[i].r].push_back(i); 
    
    vector<Server*> placed;
    for (int r = 0; r < R; ++r) {
        sort(rack_srvs[r].begin(), rack_srvs[r].end(),
             [&](int a, int b) { return servers[a].c > servers[b].c; });
        for (int j = 0; j < (int)rack_srvs[r].size(); ++j) {
            int sid = rack_srvs[r][j], p = j % P;
            servers[sid].p = p;
            pool_total[p] += servers[sid].c;
            rack_ms[p].erase(rack_ms[p].find(pool_rack[p][r]));
            pool_rack[p][r] += servers[sid].c;
            rack_ms[p].insert(pool_rack[p][r]);
            placed.push_back(&servers[sid]);
        }
    }
    
    for (int p = 0; p < P; ++p) {
        pool_gp[p] = pool_total[p] - *rack_ms[p].rbegin();
        gp_ms.insert(pool_gp[p]);
    }
    
    int n = (int)placed.size();
    if (n > 0 && P > 1) {
        mt19937 rng(1337);
        uniform_real_distribution<double> ud(0.0, 1.0);

        double t_now = 0.0, temp = 500.0;
        long long iter = 0;

        auto elapsed = [&]() {
            return chrono::duration<double>(
                chrono::high_resolution_clock::now() - t_start).count();
        };

        while (t_now < 9.2) {
            
            if (++iter % 1000 == 0) {
                t_now = elapsed();
                temp = 500.0 * pow(0.001, t_now / 9.2);  
            }

            int old_score = *gp_ms.begin();

            if (rng() % 3 != 0) {
                
                Server* sv = placed[rng() % n];
                int np = rng() % P;
                if (np == sv->p) continue;
                int op = sv->p;

                move_to(*sv, np);
                int delta = *gp_ms.begin() - old_score;
                
                if (delta < 0 && ud(rng) >= exp((double)delta / temp))
                    move_to(*sv, op);   

            } else {
                Server* s1 = placed[rng() % n];
                Server* s2 = placed[rng() % n];
                if (s1 == s2 || s1->p == s2->p) continue;
                int p1 = s1->p, p2 = s2->p;

                move_to(*s1, p2);
                move_to(*s2, p1);
                int delta = *gp_ms.begin() - old_score;
                if (delta < 0 && ud(rng) >= exp((double)delta / temp)) {
                    move_to(*s1, p1);   
                    move_to(*s2, p2);
                }
            }
        }
    }

    
    sort(servers.begin(), servers.end(),
         [](const Server& a, const Server& b) { return a.id < b.id; });
    for (auto& sv : servers) {
        if (sv.p != -1) cout << sv.r << " " << sv.s << " " << sv.p << "\n";
        else            cout << "x\n";
    }
    return 0;
}
