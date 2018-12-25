delete from m_head where id_m_head in
(select * from
	(
	select distinct id_m_head as id from m_head 
	   left join m_imu on fk_id_m_head=id_m_head 
	   group by id_m_head 
	   having count(imu_serial)=0
	) as t 
)
   