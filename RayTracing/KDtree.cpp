#include "stdafx.h"
#include "KDtree.h"

#ifdef _DEBUG
int G_cnt;
#endif

void KDtree::BorderBox::init(const Face & f)
{
	x1 = x2 = f.a->x;
	y1 = y2 = f.a->y;
	z1 = z2 = f.a->z;
	addPoint(*f.b);
	addPoint(*f.c);
}

void KDtree::BorderBox::addPoint(const Point & p)
{
	tension(x1, p.x);
	tension(y1, p.y);
	tension(z1, p.z);
	relax(x2, p.x);
	relax(y2, p.y);
	relax(z2, p.z);
}

void KDtree::BorderBox::addFace(const Face & f)
{
	addPoint(*f.a);
	addPoint(*f.b);
	addPoint(*f.c);
}

void KDtree::BorderBox::squeeze(const BorderBox & o)
{
	relax(x1, o.x1);
	relax(y1, o.y1);
	relax(z1, o.z1);
	tension(x2, o.x2);
	tension(y2, o.y2);
	tension(z2, o.z2);
}

double KDtree::BorderBox::surface() const
{
	return (x2 - x1)*(y2 - y1) + (x2 - x1)*(z2 - z1) + (y2 - y1)*(z2 - z1);
}


void KDtree::addObject(const Object & p)
{
	for (auto& i : p.f) {
		fp.push_back(&i);
	}
}

KDtree::Node * KDtree::buildTree_subtree(const std::vector<const Face*> &fp, const BorderBox &box, int height)
{
#ifdef _DEBUG
	G_cnt++;
#endif

	Node* res = new Node;
	res->direction = Node::leaf;
	res->pos = NAN;
	res->lc = res->rc = nullptr;
	res->l = (int)this->fp.size(), res->r = res->l + (int)fp.size();
	res->box = box;

	if (res->r - res->l <= FaceNumOfLeaf) {
		this->fp.insert(this->fp.end(), fp.begin(), fp.end());
		return res;
	}

	double costx, costy, costz;	//费用
	double borderx, bordery, borderz;	//边界位置
	int lnumx, lnumy, lnumz;	//分开后左边个数
	int rnumx, rnumy, rnumz;	//分开后右边个数

#pragma omp parallel sections
	{
#pragma omp section
		{
			std::tie(costx, borderx, lnumx, rnumx) = buildTree_calculate(fp, Node::x, box);
			if (res->r - res->l - lnumx <= LeastDecrease ||
				res->r - res->l - rnumx <= LeastDecrease)
				costx = INFINITY;
		}
#pragma omp section
		{
			std::tie(costy, bordery, lnumy, rnumy) = buildTree_calculate(fp, Node::y, box);
			if (res->r - res->l - lnumy <= LeastDecrease ||
				res->r - res->l - rnumy <= LeastDecrease)
				costy = INFINITY;
		}
#pragma omp section
		{
			std::tie(costz, borderz, lnumz, rnumz) = buildTree_calculate(fp, Node::z, box);
			if (res->r - res->l - lnumz <= LeastDecrease ||
				res->r - res->l - rnumz <= LeastDecrease)
				costz = INFINITY;
		}
	}

	auto buildsub = [&](double borderPos, int lnum, int rnum, Node::Direction direction) {
		if (res->r - res->l - lnum <= LeastDecrease ||
			res->r - res->l - rnum <= LeastDecrease || height > MaxKDtreeHeight) {
			std::cerr << "Large Leaf:" << res->r - res->l << std::endl;
			this->fp.insert(this->fp.end(), fp.begin(), fp.end());
			return;
		}
		res->pos = borderPos;
		res->direction = direction;

		std::vector<const Face*> lfp, rfp;
		std::tie(lfp, rfp) = buildTree_partition(fp, direction, borderPos);
		assert(lfp.size() > 1 && rfp.size() > 1);
		assert(lfp.size() == lnum && rfp.size() == rnum);

		BorderBox lbox, rbox;
		lbox.init(*lfp[0]);
		for (auto i : lfp) lbox.addFace(*i);
		lbox.squeeze(box);
		tension(lbox.b[direction], borderPos);

		rbox.init(*rfp[0]);
		for (auto i : rfp) rbox.addFace(*i);
		rbox.squeeze(box);
		relax(rbox.a[direction], borderPos);

		res->lc = buildTree_subtree(lfp, lbox, height + 1);
		res->rc = buildTree_subtree(rfp, rbox, height + 1);
	};

	if (costx <= costy && costx <= costz) {
		buildsub(borderx, lnumx, rnumx, Node::x);
	} else if (costy <= costx && costy <= costz) {
		buildsub(bordery, lnumy, rnumy, Node::y);
	} else {
		buildsub(borderz, lnumz, rnumz, Node::z);
	}

	return res;
}

std::tuple<double, double, int, int> KDtree::buildTree_calculate(const std::vector<const Face*> &fp, Node::Direction d, const BorderBox &outbox)
{
	struct BorderLine {
		double pos;
		int id;
		bool operator<(const BorderLine &b) const
		{
			if (pos == b.pos) {
				return (id & 1) && (~b.id & 1);
			}else {
				return pos < b.pos;
			}
		}
	};

	int elementnum = fp.size(), bordernum = elementnum * 2;
	std::vector<BorderLine> border(bordernum);
	std::vector<double> lcost(bordernum + 1), rcost(bordernum + 1);
	std::vector<int> lnum(bordernum + 1), rnum(bordernum + 1);

	for (int i = 0; i < elementnum; i++) {
		border[2 * i].pos = std::min({ fp[i]->a->p[d], fp[i]->b->p[d], fp[i]->c->p[d] });
		border[2 * i].id = i << 1;
		border[2 * i + 1].pos = std::max({ fp[i]->a->p[d], fp[i]->b->p[d], fp[i]->c->p[d] });
		border[2 * i + 1].id = (i << 1) | 1;
	}
	sort(border.begin(), border.end());

	int cnt = 0;
	BorderBox box;
	box.init(*fp[border[0].id >> 1]);

	lcost[0] = 0, lnum[0] = 0;
	for (int i = 1; i < bordernum; i++) {
		if (~border[i - 1].id & 1) {
			cnt++;
			box.addFace(*fp[border[i - 1].id >> 1]);
			//tension(box.b[d], (border[i - 1].pos + border[i].pos)/2);
			box.squeeze(outbox);
		}
		double tmp = box.b[d];
		tension(box.b[d], (border[i - 1].pos + border[i].pos) / 2);
		lcost[i] = cnt * box.surface();
		box.b[d] = tmp;
		lnum[i] = cnt;
	}
	lcost[bordernum] = INFINITY;

	cnt = 0;
	box.init(*fp[border[bordernum-1].id >> 1]);
	rcost[bordernum] = 0, rnum[bordernum] = 0;
	for (int i = bordernum - 1;i >= 1;i--) {
		if (border[i].id & 1) {
			cnt++;
			box.addFace(*fp[border[i].id >> 1]);
			//relax(box.a[d], (border[i].pos + border[i-1].pos)/2);
			box.squeeze(outbox);
		}
		double tmp = box.a[d];
		relax(box.a[d], (border[i].pos + border[i - 1].pos) / 2);
		rcost[i] = cnt * box.surface();
		box.a[d] = tmp;
		rnum[i] = cnt;
	}
	rcost[0] = INFINITY;
	rnum[0] = cnt;

	int bestpos = 0;
	for (int i = 1; i < bordernum; i++) if(border[i-1].pos + eps < border[i].pos){
		double pos = (border[i - 1].pos + border[i].pos) / 2;
		if (pos < outbox.a[d] || pos > outbox.b[d]) continue;
		if (lnum[i] == elementnum || rnum[i] == elementnum) continue;
		if (lcost[i] + rcost[i] < lcost[bestpos] + rcost[bestpos]) {
			bestpos = i;
		}
	}

	return std::make_tuple(
			bestpos == 0? INFINITY:lcost[bestpos] + rcost[bestpos],
			bestpos == 0? NAN:(border[bestpos - 1].pos + border[bestpos].pos)/2,
			lnum[bestpos], rnum[bestpos]
		);
}

std::tuple<std::vector<const Face*>, std::vector<const Face*>>
	KDtree::buildTree_partition(const std::vector<const Face*> &fp, Node::Direction d, double borderPos)
{
	assert(!isnan(borderPos));
	std::vector<const Face*> lvec, rvec;
	for (auto i : fp) {
		double rpos = std::max({ i->a->p[d], i->b->p[d], i->c->p[d] });
		double lpos = std::min({ i->a->p[d], i->b->p[d], i->c->p[d] });
		if (lpos < borderPos) {
			lvec.push_back(i);
		}
		if(rpos > borderPos){
			rvec.push_back(i);
		}
	}
	return std::make_tuple(lvec, rvec);
}

void KDtree::buildTree()
{
	std::vector<const Face*> orifp = std::move(fp);
	fp.clear();

#ifdef _DEBUG
	G_cnt = 0;
#endif

	BorderBox box;
	box.x1 = box.y1 = box.z1 = -INFINITY;
	box.x2 = box.y2 = box.z2 = INFINITY;

	root = buildTree_subtree(orifp, box, 0);

#ifdef _DEBUG
	std::cerr << "buildtree:" << G_cnt << std::endl;
#endif
}


std::tuple<const Face*, double> KDtree::queryNode(const Node* node, const Point & s, const Point & dir, const Face* ignore) const
{
	if (node->direction == Node::leaf) {
		double bestt = INFINITY;
		const Face* res = nullptr;
		for (int i = node->l; i < node->r; i++) {
			if (fp[i] == ignore) continue;
#ifdef _DEBUG
			G_cnt++;
#endif
			double t = queryIntersectTime(*fp[i], s, dir);
			assert(t >= 0);
#ifdef _DEBUG
			if (t < eps)
				std::cerr << "warning: some faces is too close to startPos" << std::endl;
#endif
			if (t> eps && t < bestt) {
				res = fp[i], bestt = t;
			}
		}
		return std::make_tuple(res, bestt);
	}

	double leftt = queryIntersectTime(node->lc->box, s, dir);
	double rightt = queryIntersectTime(node->rc->box, s, dir);
	if (isinf(leftt) && isinf(rightt)) return std::make_tuple(nullptr, INFINITY);
	if (leftt < rightt) {
		auto res = queryNode(node->lc, s, dir, ignore);
		if (std::get<0>(res)) return res;
		if(!isinf(rightt)) return queryNode(node->rc, s, dir, ignore);
	}else{
		auto res = queryNode(node->rc, s, dir, ignore);
		if (std::get<0>(res)) return res;
		if(!isinf(leftt)) return queryNode(node->lc, s, dir, ignore);
	}
	return std::make_tuple(nullptr, INFINITY);
}

std::tuple<const Face*, Point> KDtree::query(const Point & s, const Point & dir, const Face* ignore/* = nullptr*/) const
{
#ifdef _DEBUG
	G_cnt = 0;
#endif
	auto res = queryNode(root, s, dir, ignore);
#ifdef _DEBUG
	//std::cerr << "query:" << G_cnt << std::endl;
#endif
	return std::make_tuple(std::get<0>(res), std::get<1>(res) * dir + s);
}

std::tuple<const Face*, Point> KDtree::queryBF(const Point & s, const Point & dir, const Face * ignore) const
{
	double bestt = INFINITY;
	const Face* res = nullptr;
	for (int i = root->l; i < root->r; i++) {
		if (fp[i] == ignore) continue;
		double t = queryIntersectTime(*fp[i], s, dir);
		assert(t >= 0);
		if (t < eps)
			std::cerr << "warning: some faces is too close to startPos" << std::endl;
		else if (t < bestt) {
			res = fp[i], bestt = t;
		}
	}
	return std::make_tuple(res, s + dir * bestt);
}

double queryIntersectTime(const KDtree::BorderBox & box, const Point & s, const Point & dir)
{
	double tx1 = (box.x1 - s.x) / dir.x;
	double tx2 = (box.x2 - s.x) / dir.x;
	double ty1 = (box.y1 - s.y) / dir.y;
	double ty2 = (box.y2 - s.y) / dir.y;
	double tz1 = (box.z1 - s.z) / dir.z;
	double tz2 = (box.z2 - s.z) / dir.z;
	if (tx1 > tx2) std::swap(tx1, tx2);
	if (ty1 > ty2) std::swap(ty1, ty2);
	if (tz1 > tz2) std::swap(tz1, tz2);
	double t1 = std::max({ tx1, ty1, tz1 });
	double t2 = std::min({ tx2, ty2, tz2 });

	if (t2 < t1) return INFINITY;

	if (t2 > 0) {
		if (t1 > 0) return t1;
		else return 0;
	}
	return INFINITY;
}
