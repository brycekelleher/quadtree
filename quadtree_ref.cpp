typedef struct qtnode_s
{
	struct qtnode_s* m_parent;
	struct qtnode_s* m_child[4];

} qtnode_t;

int NumNodesForLevel(int level)
{
	int count;

	count = 1;
	for(i = 0; i < level; i++)
		count *= 4;

	return count;
}

void SetupPointersBreadth_r(qtnode_t * qnodes, int level, int numlevels)
{
	int i;

	if(level == numlevels)
		return;

	count = NumNodesForLevel(level);
	for(i = 0; i < count; i++)
	{
		qnodes[i].m_child[0] = &qnodes[count + (i * 4) + 0];
		qnodes[i].m_child[1] = &qnodes[count + (i * 4) + 1];
		qnodes[i].m_child[2] = &qnodes[count + (i * 4) + 2];
		qnodes[i].m_child[3] = &qnodes[count + (i * 4) + 3];

		qnodes[count + (i * 4) + 0].m_parent = &qnodes[i];
		qnodes[count + (i * 4) + 1].m_parent = &qnodes[i];
		qnodes[count + (i * 4) + 2].m_parent = &qnodes[i];
		qnodes[count + (i * 4) + 3].m_parent = &qnodes[i];
	}

	// Recurse through the other levels
	SetupPointersBreadth_r(qnodes + count, level + 1, numlevels);
}

static int s_numnodes;
static qnodes_t* s_nodes;

// Could also allocate all four nodes at once at then recurse?
void SetupPointersDepth_r(qnode_t* node, int level, int numlevels)
{
	int i;

	if(level == numlevels)
		return;

	for(i = 0; i < 4; i++)
	{
		node->m_child[i] = s_qnodes + s_numnodes;
		node->m_child[i]->m_parent = node;
		s_numnodes++;

		// Recurse this node
		SetupPointersDepth_r(node->m_child[i], level + 1, numlevels);
	}
}

void SetupPointersDepth(int numlevels)
{
	s_numnodes = 1;
	SetupPointersDepth_r(s_nodes, 0, numlevels);
}

